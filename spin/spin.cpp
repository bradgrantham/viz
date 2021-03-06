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
#include "loader.h"

using namespace std;

//------------------------------------------------------------------------

static bool gDrawWireframe = false;
static bool gStreamFrames = false;

static int gWindowWidth;
static int gWindowHeight;

static double gMotionReported = false;
static double gOldMouseX, gOldMouseY;

// XXX Allow these to be set by options
bool gVerbose = false;

// XXX Controller needs ability to set camera projection...
const float gFOV = 45; // XXX XXX also gFOV in DefaultController...

NodePtr gSceneRoot;
ControllerPtr gSceneController;

bool ExactlyEqual(const mat4f&m1, const mat4f&m2)
{
    for(int i = 0; i < 16; i++)
        if(m1[i] != m2[i])
            return false;
    return true;
}

void DrawScene(float now)
{
    float nearClip, farClip;

    /* XXX - need to create new box from all subordinate boxes */
    nearClip = .1 ; // XXX - gSceneManip->m_translation[2] - gSceneManip->m_reference_size;
    farClip = 1000 ; // XXX - gSceneManip->m_translation[2] + gSceneManip->m_reference_size;
    // nearClip = std::max(nearClip, 0.1 * gSceneManip->m_reference_size);
    // farClip = std::min(farClip, 2 * gSceneManip->m_reference_size);

    // XXX Calculation of frustum matrix will move into a Node subclass
    float frustumLeft, frustumRight, frustumBottom, frustumTop;
    frustumTop = tanf(gFOV / 180.0 * 3.14159 / 2) * nearClip;
    frustumBottom = -frustumTop;
    frustumRight = frustumTop * gWindowWidth / gWindowHeight;
    frustumLeft = -frustumRight;
    mat4f tmp_projection = mat4f::frustum(frustumLeft, frustumRight, frustumBottom, frustumTop, nearClip, farClip);

    Light light(vec4f(.577, .577, .577, 0), vec4f(1, 1, 1, 1));

    vector<Light> lights;
    lights.push_back(light);
    Environment env(tmp_projection, mat4f::identity, lights);
    DisplayList displaylist;
    gSceneRoot->Visit(env, displaylist);

    mat4f projection;
    mat4f modelview;
    bool loadMatrices = true;

    GLuint program = 0;

    for(auto it : displaylist) {
        DisplayInfo displayinfo = it.first;
        vector<DrawablePtr>& drawables = it.second;
        EnvironmentUniforms& envu = displayinfo.envu;

        if(program != displayinfo.program) {
            glUseProgram(displayinfo.program);
            loadMatrices = true;

            // XXX Should be loaded from environment
            glUniform4fv(envu.lightPosition, 1, lights[0].position.m_v);
            glUniform4fv(envu.lightColor, 1, lights[0].color.m_v);
            CheckOpenGL(__FILE__, __LINE__);

            program = displayinfo.program;
        }

        if(loadMatrices || !ExactlyEqual(modelview, displayinfo.modelview)) {
            mat4f modelview_normal = displayinfo.modelview;
            // XXX might not invert every time
            // XXX parallel normal matrix math path?
            modelview_normal.transpose();
            modelview_normal.invert();

            glUniformMatrix4fv(envu.modelview, 1, GL_FALSE, displayinfo.modelview.m_v);
            glUniformMatrix4fv(envu.modelviewNormal, 1, GL_FALSE, modelview_normal.m_v);
            modelview = displayinfo.modelview;
        }

        if(loadMatrices || !ExactlyEqual(projection, displayinfo.projection)) {
            glUniformMatrix4fv(envu.projection, 1, GL_FALSE, displayinfo.projection.m_v);
            projection = displayinfo.projection;
        }

        loadMatrices = false;

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

static void InitializeScene(NodePtr& gSceneRoot)
{
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

            default:
                bool quit = gSceneController->Key(key, scancode, action, mods);
                if(quit)
                    glfwSetWindowShouldClose(window, GL_TRUE);
                break;
        }
    }
}

static void ResizeCallback(GLFWwindow *window, int x, int y)
{
    glfwGetFramebufferSize(window, &gWindowWidth, &gWindowHeight);
    glViewport(0, 0, gWindowWidth, gWindowHeight);
    gSceneController->Resize(gWindowWidth, gWindowHeight);
}

static void ButtonCallback(GLFWwindow *window, int b, int action, int mods)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    gOldMouseX = x;
    gOldMouseY = y;

    bool quit = gSceneController->Button(b, action, mods, x, y);
    if(quit)
        glfwSetWindowShouldClose(window, GL_TRUE);
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

    bool quit = gSceneController->Motion(dx, dy);
    if(quit)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void ScrollCallback(GLFWwindow *window, double dx, double dy)
{
    bool quit = gSceneController->Scroll(dx, dy);
    if(quit)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

GroupPtr gSceneGroup;
chrono::time_point<chrono::system_clock> gSceneStartTime;
chrono::time_point<chrono::system_clock> gScenePreviousTime;

static void DrawFrame(GLFWwindow *window)
{
    CheckOpenGL(__FILE__, __LINE__);

    chrono::time_point<chrono::system_clock> now =
        chrono::system_clock::now();
    chrono::duration<float> elapsed_seconds = now - gSceneStartTime;
    float elapsed = elapsed_seconds.count();
    gSceneController->Update(elapsed);
    gScenePreviousTime = now;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CheckOpenGL(__FILE__, __LINE__);

    DrawScene(elapsed);

    CheckOpenGL(__FILE__, __LINE__);
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
    bool success;
    tie(success, gSceneRoot, gSceneController) = LoadScene(scene_filename);
    if(!success) {
        fprintf(stderr, "couldn't load scene from %s\n", scene_filename);
        exit(EXIT_FAILURE);
    }
    InitializeScene(gSceneRoot);

    if(gVerbose) {
        printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
        printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
    }

    gSceneController->Resize(gWindowWidth, gWindowHeight);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, ButtonCallback);
    glfwSetCursorPosCallback(window, MotionCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetFramebufferSizeCallback(window, ResizeCallback);
    glfwSetWindowRefreshCallback(window, DrawFrame);

    gSceneStartTime = gScenePreviousTime = chrono::system_clock::now();

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
