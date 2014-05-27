//
// Copyright 2013-2014, Bradley A. Grantham
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <limits>
#include <algorithm>
#include <vector>
#include <unistd.h>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include "vectormath.h"
#include "geometry.h"
#include "manipulator.h"

#include "drawable.h"
#include "builtin_loader.h"
#include "trisrc_loader.h"

//------------------------------------------------------------------------

static manipulator *gSceneManip;
static manipulator *gCurrentManip = NULL;

static bool gDrawWireframe = false;
static bool gStreamFrames = false;

static int gWindowWidth;
static int gWindowHeight;

static double gMotionReported = false;
static double gOldMouseX, gOldMouseY;
static int gButtonPressed = -1;

// XXX Allow these to be set by options
bool gVerbose = true;

float gFOV = 45;

struct Light
{
    vec4f position;
    vec4f color;
    Light(const vec4f& position_, const vec4f color_) :
        position(position_),
        color(color_)
    {}
};

struct Environment
{
    mat4f projection;
    mat4f modelview;
    std::vector<Light> lights; // only one supported at present

    Environment(const mat4f& projection_, const mat4f& modelview_, const std::vector<Light>& lights_) :
        projection(projection_),
        modelview(modelview_),
        lights(lights_)
    {}
};

struct DisplayInfo
{
    mat4f modelview;
    GLuint program;
    EnvironmentUniforms envu; // XXX Clumsy?  Really tied to program, so shouldn't be independent here

    DisplayInfo(const mat4f& modelview_, GLuint program_, const EnvironmentUniforms& envu_) :
        modelview(modelview_),
        program(program_),
        envu(envu_)
    {}

    struct Comparator
    {
        bool operator() (const DisplayInfo& d1, const DisplayInfo& d2) const
        {
            for(int i = 0; i < 16; i++) {
                if(d1.modelview.m_v[i] < d2.modelview.m_v[i])
                    return true;
                if(d1.modelview.m_v[i] < d2.modelview.m_v[i])
                    return false;
            }
            if(d1.program < d2.program)
                return true;
            if(d1.program < d2.program)
                return false;
            return false;
        }
    };

};
typedef std::map<DisplayInfo, std::vector<Drawable::sptr>, DisplayInfo::Comparator> DisplayList;

struct Node
{
    typedef boost::shared_ptr<Node> sptr;
    box bounds; // Later can cull
    virtual void Visit(const Environment& env, DisplayList& displaylist) = 0;

    Node(const box& bounds_) :
        bounds(bounds_)
    {}
    virtual ~Node() {}
};

struct Shape : public Node
{
    typedef boost::shared_ptr<Shape> sptr;
    Drawable::sptr drawable;
    virtual void Visit(const Environment& env, DisplayList& displaylist);
    Shape(Drawable::sptr& drawable_) :
        Node(drawable_->bounds),
        drawable(drawable_)
    {}
    virtual ~Shape() {}
};

void Shape::Visit(const Environment& env, DisplayList& displaylist)
{
    displaylist[DisplayInfo(env.modelview, drawable->GetProgram(), drawable->GetEnvironmentUniforms())].push_back(drawable);
}

box TransformedBounds(const mat4f& transform, std::vector<Node::sptr> children)
{
    box b;

    for(auto it = children.begin(); it != children.end(); it++)
        b.extend((*it)->bounds * transform);

    return b;
}

struct Group : public Node 
{
    typedef boost::shared_ptr<Group> sptr;
    mat4f transform;
    std::vector<Node::sptr> children;

    virtual void Visit(const Environment& env, DisplayList& displaylist);

    Group(const mat4f& transform_, std::vector<Node::sptr> children_) :
        Node(TransformedBounds(transform_, children_)),
        transform(transform_),
        children(children_)
    {}

    Group(std::vector<Node::sptr> children_) :
        Node(TransformedBounds(mat4f::identity, children_)),
        transform(mat4f::identity),
        children(children_)
    {}

    virtual ~Group() {}
};

void Group::Visit(const Environment& env, DisplayList& displaylist)
{
    mat4f newtransform = transform * env.modelview;
    Environment env2(env.projection, newtransform, env.lights);
    for(auto it = children.begin(); it != children.end(); it++)
        (*it)->Visit(env2, displaylist);
}

Node::sptr gSceneRoot;

bool ExactlyEqual(const mat4f&m1, const mat4f&m2)
{
    for(int i = 0; i < 16; i++)
        if(m1[i] != m2[i])
            return false;
    return true;
}

void DrawScene()
{
    float nearClip, farClip;

    /* XXX - need to create new box from all subordinate boxes */
    nearClip = - gSceneManip->m_translation[2] - gSceneManip->m_reference_size;
    farClip = - gSceneManip->m_translation[2] + gSceneManip->m_reference_size;
    if(nearClip < 0.1 * gSceneManip->m_reference_size)
	nearClip = 0.1 * gSceneManip->m_reference_size;
    if(farClip < 0.2 * gSceneManip->m_reference_size)
	nearClip = 0.2 * gSceneManip->m_reference_size;

    float frustumLeft, frustumRight, frustumBottom, frustumTop;
    frustumTop = tanf(gFOV / 180.0 * 3.14159 / 2) * nearClip;
    frustumBottom = -frustumTop;
    frustumRight = frustumTop * gWindowWidth / gWindowHeight;
    frustumLeft = -frustumRight;
    mat4f projection = mat4f::frustum(frustumLeft, frustumRight, frustumBottom, frustumTop, nearClip, farClip);

    Light light(vec4f(.577, .577, .577, 0), vec4f(1, 1, 1, 1));

    std::vector<Light> lights;
    lights.push_back(light);
    Environment env(projection, gSceneManip->m_matrix, lights);
    DisplayList displaylist;
    gSceneRoot->Visit(env, displaylist);

    mat4f modelview;
    bool modelviewInvalid = true;
    GLuint program = 0;
    for(auto it = displaylist.begin(); it != displaylist.end(); it++) {
        DisplayInfo di = it->first;
        auto drawables = it->second;
        if(program != di.program) {
            glUseProgram(di.program);
            modelviewInvalid = true;
            glUniformMatrix4fv(di.envu.projection, 1, GL_FALSE, projection.m_v);
            glUniform4fv(di.envu.lightPosition, 1, lights[0].position.m_v);
            glUniform4fv(di.envu.lightColor, 1, lights[0].color.m_v);
            CheckOpenGL(__FILE__, __LINE__);
            program = di.program;
        }
        if(modelviewInvalid || !ExactlyEqual(modelview, di.modelview)) {
            modelviewInvalid = false;
            mat4f modelview_normal = di.modelview;
            // XXX should not invert every time
            // XXX parallel normal matrix math path?
            modelview_normal.transpose();
            modelview_normal.invert();

            glUniformMatrix4fv(di.envu.modelview, 1, GL_FALSE, di.modelview.m_v);
            glUniformMatrix4fv(di.envu.modelviewNormal, 1, GL_FALSE, modelview_normal.m_v);
            modelview = di.modelview;
        }
        for(auto it2 = drawables.begin(); it2 != drawables.end(); it2++) {
            (*it2)->Draw(0.0f, gDrawWireframe);
        }
    }
}

void InitializeGL()
{
    CheckOpenGL(__FILE__, __LINE__);
    glClearColor(.25, .25, .25, 0);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    CheckOpenGL(__FILE__, __LINE__);
}

void TeardownGL()
{
}

static void InitializeScene(Node::sptr& gSceneRoot)
{
    gSceneManip = new manipulator(gSceneRoot->bounds, gFOV / 180.0 * 3.14159);

    gCurrentManip = gSceneManip;
}

static void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW: %s\n", description);
}

static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS) {
        switch(key) {
            case 'W':
                gDrawWireframe = !gDrawWireframe;
                break;

            case 'R':
                gCurrentManip->m_mode = manipulator::ROTATE;
                break;

            case 'O':
                gCurrentManip->m_mode = manipulator::ROLL;
                break;

            case 'X':
                gCurrentManip->m_mode = manipulator::SCROLL;
                break;

            case 'Z':
                gCurrentManip->m_mode = manipulator::DOLLY;
                break;

            case 'Q': case '\033':
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    }
}

static void ResizeCallback(GLFWwindow *window, int x, int y)
{
    glfwGetFramebufferSize(window, &gWindowWidth, &gWindowHeight);
    glViewport(0, 0, gWindowWidth, gWindowHeight);
}

static void ButtonCallback(GLFWwindow *window, int b, int action, int mods)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if(b == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
        gButtonPressed = 1;
	gOldMouseX = x;
	gOldMouseY = y;
    } else {
        gButtonPressed = -1;
    }
}

static void MotionCallback(GLFWwindow *window, double x, double y)
{
    // glfw/glfw#103
    // If no motion has been reported yet, we catch the first motion
    // reported and store the current location
    if(!gMotionReported) {
        gMotionReported = true;
        gOldMouseX = x;
        gOldMouseY = y;
    }

    double dx, dy;

    dx = x - gOldMouseX;
    dy = y - gOldMouseY;

    gOldMouseX = x;
    gOldMouseY = y;

    if(gButtonPressed == 1) {
        gCurrentManip->move(dx / gWindowWidth, dy / gWindowHeight);
    }
}

static void ScrollCallback(GLFWwindow *window, double dx, double dy)
{
    gCurrentManip->move(dx / gWindowWidth, dy / gWindowHeight);
}

static void DrawFrame(GLFWwindow *window)
{
    CheckOpenGL(__FILE__, __LINE__);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CheckOpenGL(__FILE__, __LINE__);

    DrawScene();

    CheckOpenGL(__FILE__, __LINE__);
}

Node::sptr LoadScene(const std::string& filename)
{
    int index = filename.find_last_of(".");
    std::string extension = filename.substr(index + 1);

    std::vector<Drawable::sptr> objects;

    if(extension == "builtin") {

        bool success = BuiltinLoader::Load(filename, objects);
        if(!success)
            return Node::sptr();

    } else if(extension == "trisrc") {

        bool success = TriSrcLoader::Load(filename, objects);
        if(!success)
            return Node::sptr();

    } else {

        return Node::sptr();
    }

    std::vector<Node::sptr> nodes;
    for(auto it = objects.begin(); it != objects.end(); it++) {
        auto db = *it;
        Shape::sptr shape(new Shape(db));
        nodes.push_back(Shape::sptr(new Shape(*it)));
    }
    return Group::sptr(new Group(mat4f::identity, nodes));
}

int main(int argc, char **argv)
{
    const char *progname = argv[0];
    if(argc < 2) {
        fprintf(stderr, "usage: %s filename # e.g. \"%s 64gon.builtin\"\n", progname, progname);
        exit(EXIT_FAILURE);
    }

    const char *scene_filename = argv[1];

    GLFWwindow* window;

    glfwSetErrorCallback(ErrorCallback);

    if(!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

    glfwWindowHint(GLFW_SAMPLES, 4);
    window = glfwCreateWindow(gWindowWidth = 512, gWindowHeight = 512, "Spin", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stdout, "Couldn't open main window\n");
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    InitializeGL();
    gSceneRoot = LoadScene(scene_filename);
    if(!gSceneRoot) {
        fprintf(stderr, "couldn't load scene from %s\n", scene_filename);
        exit(EXIT_FAILURE);
    }
    InitializeScene(gSceneRoot);

    if(gVerbose) {
        printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
        printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
    }

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, ButtonCallback);
    glfwSetCursorPosCallback(window, MotionCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetFramebufferSizeCallback(window, ResizeCallback);
    glfwSetWindowRefreshCallback(window, DrawFrame);

    while (!glfwWindowShouldClose(window)) {

        DrawFrame(window);

        glfwSwapBuffers(window);

        if(gStreamFrames)
            glfwPollEvents();
        else
            glfwWaitEvents();
    }

    glfwTerminate();
}
