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
#include <chrono>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include "vectormath.h"
#include "geometry.h"
#include "manipulator.h"

#include "drawable.h"
#include "builtin_loader.h"
#include "trisrc_loader.h"

using namespace std;

//------------------------------------------------------------------------

static manipulator *gSceneManip;
static manipulator *gCurrentManip = nullptr;

static bool gDrawWireframe = false;
static bool gStreamFrames = false;

static int gWindowWidth;
static int gWindowHeight;

static double gMotionReported = false;
static double gOldMouseX, gOldMouseY;
static int gButtonPressed = -1;

// XXX Allow these to be set by options
bool gVerbose = false;

float gFOV = 45;

Node::sptr gSceneRoot;

bool ExactlyEqual(const mat4f&m1, const mat4f&m2)
{
    for(int i = 0; i < 16; i++)
        if(m1[i] != m2[i])
            return false;
    return true;
}

void DrawScene(double now)
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

    vector<Light> lights;
    lights.push_back(light);
    Environment env(projection, gSceneManip->m_matrix, lights);
    DisplayList displaylist;
    gSceneRoot->Visit(env, displaylist);

    mat4f modelview;
    bool modelviewInvalid = true;
    GLuint program = 0;
    for(auto it : displaylist) {
        DisplayInfo displayinfo = it.first;
        vector<Drawable::sptr>& drawables = it.second;
        EnvironmentUniforms& envu = displayinfo.envu;
        if(program != displayinfo.program) {
            glUseProgram(displayinfo.program);
            modelviewInvalid = true;
            glUniformMatrix4fv(envu.projection, 1, GL_FALSE, projection.m_v);
            glUniform4fv(envu.lightPosition, 1, lights[0].position.m_v);
            glUniform4fv(envu.lightColor, 1, lights[0].color.m_v);
            CheckOpenGL(__FILE__, __LINE__);
            program = displayinfo.program;
        }
        if(modelviewInvalid || !ExactlyEqual(modelview, displayinfo.modelview)) {
            modelviewInvalid = false;
            mat4f modelview_normal = displayinfo.modelview;
            // XXX should not invert every time
            // XXX parallel normal matrix math path?
            modelview_normal.transpose();
            modelview_normal.invert();

            glUniformMatrix4fv(envu.modelview, 1, GL_FALSE, displayinfo.modelview.m_v);
            glUniformMatrix4fv(envu.modelviewNormal, 1, GL_FALSE, modelview_normal.m_v);
            modelview = displayinfo.modelview;
        }
        for(auto drawable : drawables) {
            drawable->Draw(now, gDrawWireframe);
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

Group::sptr gSceneGroup;
chrono::time_point<chrono::system_clock> gSceneStartTime;
chrono::time_point<chrono::system_clock> gScenePreviousTime;

static void DrawFrame(GLFWwindow *window)
{
    CheckOpenGL(__FILE__, __LINE__);

    chrono::time_point<chrono::system_clock> now =
        chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = now - gSceneStartTime;
    double elapsed = elapsed_seconds.count();
    gScenePreviousTime = now;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CheckOpenGL(__FILE__, __LINE__);

    DrawScene(elapsed);

    CheckOpenGL(__FILE__, __LINE__);
}

Node::sptr LoadScene(const string& filename)
{
    int index = filename.find_last_of(".");
    string extension = filename.substr(index + 1);

    Node::sptr root;

    if(extension == "builtin") {

        bool success;
        tie(success, root) = BuiltinLoader::Load(filename);
        if(!success)
            root = Node::sptr();

    } else if(extension == "trisrc") {


        bool success;
        tie(success, root) = TriSrcLoader::Load(filename);
        if(!success)
            return Node::sptr();
    

    } else {

        root = Node::sptr();
    }

    gSceneStartTime = gScenePreviousTime = chrono::system_clock::now();

    return root;
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
