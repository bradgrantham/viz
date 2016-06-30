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
#include <limits>
#include <algorithm>
#include <unistd.h>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include "vectormath.h"
#include "geometry.h"
#include "manipulator.h"

#include "drawable.h"

//------------------------------------------------------------------------

static manipulator *gSceneManip;
static manipulator *gObjectManip;
static manipulator *gCurrentManip = NULL;

static bool gDrawWireframe = false;

static bool gStreamFrames = false;

static int gWindowWidth;
static int gWindowHeight;

static bool gMotionReported = false;

static double gOldMouseX, gOldMouseY;
static int gButtonPressed = -1;

// XXX Allow these to be set by options
bool gPrintShaderLog = true;
bool gVerbose = true;

//----------------------------------------------------------------------------
// Actual GL functions

static void CheckOpenGL(const char *filename, int line)
{
    int glerr;

    if((glerr = glGetError()) != GL_NO_ERROR) {
        printf("GL Error: %04X at %s:%d\n", glerr, filename, line);
    }
}

static bool CheckShaderCompile(GLuint shader, const std::string& shader_name)
{
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_TRUE)
	return true;

    if(gPrintShaderLog) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        if (length > 0) {
            char log[length];
            glGetShaderInfoLog(shader, length, NULL, log);
            fprintf(stderr, "%s shader error log:\n%s\n", shader_name.c_str(), log);
        }

        fprintf(stderr, "%s compile failure.\n", shader_name.c_str());
        fprintf(stderr, "shader text:\n");
        glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &length);
        char source[length];
        glGetShaderSource(shader, length, NULL, source);
        fprintf(stderr, "%s\n", source);
    }
    return false;
}

static bool CheckProgramLink(GLuint program)
{
    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status == GL_TRUE)
	return true;

    if(gPrintShaderLog) {
        int log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

        if (log_length > 0) {
            char log[log_length];
            glGetProgramInfoLog(program, log_length, NULL, log);
            fprintf(stderr, "program error log: %s\n",log);
        }
    }

    return false;
}

GLuint gVertexArray;
GLuint gVertexBuffer;

GLuint gModelviewUniform;
GLuint gModelviewNormalUniform;
GLuint gProjectionUniform;

GLuint gMaterialDiffuseUniform;
GLuint gMaterialAmbientUniform;
GLuint gMaterialSpecularUniform;
GLuint gMaterialShininessUniform;

GLuint gLightPositionUniform;
GLuint gLightColorUniform;

const int kPositionAttrib = 0; 
const int kNormalAttrib = 1; 

GLuint gProgram;

float gFOV = 45;

static const char *gVertexShaderText = "\n\
    uniform mat4 modelview_matrix;\n\
    uniform mat4 modelview_normal_matrix;\n\
    uniform mat4 projection_matrix;\n\
    in vec3 position;\n\
    in vec3 normal;\n\
    \n\
    out vec3 vertex_normal;\n\
    out vec4 vertex_position;\n\
    out vec3 eye_direction;\n\
    \n\
    void main()\n\
    {\n\
    \n\
        vertex_normal = (modelview_normal_matrix * vec4(normal, 0.0)).xyz;\n\
        vertex_position = modelview_matrix * vec4(position, 1.0);\n\
        eye_direction = -vertex_position.xyz;\n\
    \n\
        gl_Position = projection_matrix * modelview_matrix * vec4(position, 1.0);\n\
        ;\n\
    }\n";


static const char *gFragmentShaderText = "\n\
    uniform vec4 material_diffuse;\n\
    uniform vec4 material_specular;\n\
    uniform vec4 material_ambient;\n\
    uniform float material_shininess;\n\
    \n\
    uniform vec4 light_position;\n\
    uniform vec4 light_color;\n\
    \n\
    in vec3 vertex_normal;\n\
    in vec4 vertex_position;\n\
    in vec3 eye_direction;\n\
    out vec4 color;\n\
    \n\
    vec3 unitvec(vec4 p1, vec4 p2)\n\
    {\n\
        if(p1.w == 0 && p2.w == 0)\n\
            return vec3(p2 - p1);\n\
        if(p1.w == 0)\n\
            return vec3(-p1);\n\
        if(p2.w == 0)\n\
            return vec3(p2);\n\
        return p2.xyz / p2.w - p1.xyz / p1.w;\n\
    }\n\
    \n\
    void main()\n\
    {\n\
        vec3 normal = normalize(vertex_normal);\n\
    \n\
        int light;\n\
        vec3 edir = normalize(eye_direction);\n\
    \n\
        vec4 light_pos = light_position;\n\
        vec3 ldir = normalize(unitvec(vertex_position, light_pos));\n\
        vec3 refl = reflect(-ldir, normal);\n\
\n\
        vec4 diffuse = max(0, dot(normal, ldir)) * light_color * .8;\n\
        vec4 ambient = light_color * .2;\n\
        vec4 specular = pow(max(0, dot(refl, edir)), material_shininess) * light_color * .8;\n\
    \n\
        color = diffuse * material_diffuse + ambient * material_ambient + specular * material_specular;\n\
    }\n";

static GLuint GenerateProgram()
{
    std::string spec_string;

    spec_string = "#version 140\n";

    // reset line number so that I can view errors with the line number
    // they have in the base shaders.
    spec_string += "#line 0\n";

    std::string vertex_shader_string = spec_string + gVertexShaderText;
    std::string fragment_shader_string = spec_string + gFragmentShaderText;

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char *string = vertex_shader_string.c_str();
    glShaderSource(vertex_shader, 1, &string, NULL);
    glCompileShader(vertex_shader);
    if(!CheckShaderCompile(vertex_shader, "vertex shader"))
	return 0;

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    string = fragment_shader_string.c_str();
    glShaderSource(fragment_shader, 1, &string, NULL);
    glCompileShader(fragment_shader);
    if(!CheckShaderCompile(fragment_shader, "fragment shader"))
	return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    // XXX Really need to do this generically
    glBindAttribLocation(program, kPositionAttrib, "position");
    glBindAttribLocation(program, kNormalAttrib, "normal");
    CheckOpenGL(__FILE__, __LINE__);

    glLinkProgram(program);
    CheckOpenGL(__FILE__, __LINE__);
    if(!CheckProgramLink(program))
	return 0;

    return program;
}

void InitializeObject()
{
    glGenVertexArrays(1, &gVertexArray);
    glBindVertexArray(gVertexArray);
    CheckOpenGL(__FILE__, __LINE__);

    glGenBuffers(1, &gVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * gTriangleCount * 3, gVertices, GL_STATIC_DRAW);
    CheckOpenGL(__FILE__, __LINE__);

    size_t stride = sizeof(Vertex);
    size_t normalOffset = sizeof(float) * 3;

    glVertexAttribPointer(kPositionAttrib, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(kPositionAttrib);

    glVertexAttribPointer(kNormalAttrib, 3, GL_FLOAT, GL_FALSE, stride, (void*)normalOffset);
    glEnableVertexAttribArray(kNormalAttrib);
    CheckOpenGL(__FILE__, __LINE__);

    glBindVertexArray(GL_NONE);
}

void DrawObject(float objectTime, bool drawWireframe)
{
    CheckOpenGL(__FILE__, __LINE__);

    static float objectDiffuse[4] = {.8, .7, .6, 1};
    static float objectSpecular[4] = {1, 1, 1, 1};
    static float objectShininess = 50;

    glUniform4fv(gMaterialAmbientUniform, 1, objectDiffuse);
    glUniform4fv(gMaterialDiffuseUniform, 1, objectDiffuse);
    glUniform4fv(gMaterialSpecularUniform, 1, objectSpecular);
    glUniform1f(gMaterialShininessUniform, objectShininess);
    CheckOpenGL(__FILE__, __LINE__);

    glBindVertexArray(gVertexArray);
    CheckOpenGL(__FILE__, __LINE__);

    if(drawWireframe) {
        for(int i = 0; i < gTriangleCount; i++)
            glDrawArrays(GL_LINE_LOOP, i * 3, 3);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, gTriangleCount * 3);
    }

    CheckOpenGL(__FILE__, __LINE__);
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
    glUniformMatrix4fv(gProjectionUniform, 1, GL_FALSE, projection.m_v);
    CheckOpenGL(__FILE__, __LINE__);

    float lightPosition[4] = {0, 0, 1, 0};
    float lightColor[4] = {1, 1, 1, 1};
    glUniform4fv(gLightPositionUniform, 1, lightPosition);
    glUniform4fv(gLightColorUniform, 1, lightColor);
    CheckOpenGL(__FILE__, __LINE__);

    /* draw floor, draw shadow, etc */

    mat4f modelview = gObjectManip->m_matrix * gSceneManip->m_matrix;
    mat4f modelview_normal = modelview;
    // XXX might not invert every time; parallel normal matrix math path?
    modelview_normal.transpose();
    modelview_normal.invert();
    glUniformMatrix4fv(gModelviewUniform, 1, GL_FALSE, modelview.m_v);
    glUniformMatrix4fv(gModelviewNormalUniform, 1, GL_FALSE, modelview_normal.m_v);
    DrawObject(0, gDrawWireframe);
}


void InitializeGL()
{
    CheckOpenGL(__FILE__, __LINE__);
    glClearColor(.25, .25, .25, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    gProgram = GenerateProgram();
    CheckOpenGL(__FILE__, __LINE__);

    glUseProgram(gProgram);
    CheckOpenGL(__FILE__, __LINE__);

    gMaterialDiffuseUniform = glGetUniformLocation(gProgram, "material_diffuse");
    gMaterialSpecularUniform = glGetUniformLocation(gProgram, "material_specular");
    gMaterialAmbientUniform = glGetUniformLocation(gProgram, "material_ambient");
    gMaterialShininessUniform = glGetUniformLocation(gProgram, "material_shininess");

    gLightPositionUniform = glGetUniformLocation(gProgram, "light_position");
    gLightColorUniform = glGetUniformLocation(gProgram, "light_color");

    gModelviewUniform = glGetUniformLocation(gProgram, "modelview_matrix");
    gModelviewNormalUniform = glGetUniformLocation(gProgram, "modelview_normal_matrix");
    gProjectionUniform = glGetUniformLocation(gProgram, "projection_matrix");

    CheckOpenGL(__FILE__, __LINE__);
}

static void InitializeModel()
{
    box bounds;
    for(int i = 0; i < gTriangleCount * 3; i++)
        bounds.extend(gVertices[i].v);

    gSceneManip = new manipulator(bounds, gFOV / 180.0 * 3.14159);

    gObjectManip = new manipulator();
    gObjectManip->calculate_matrix();

    gCurrentManip = gSceneManip;

    InitializeObject();
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

            case '1':
                gCurrentManip = gSceneManip;
                break;

            case '2':
                gCurrentManip = gObjectManip;
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
        if(gCurrentManip == gSceneManip)
            gObjectManip->set_frame(gSceneManip->m_matrix);
    }
}

static void ScrollCallback(GLFWwindow *window, double dx, double dy)
{
    gCurrentManip->move(dx / gWindowWidth, dy / gWindowHeight);
    if(gCurrentManip == gSceneManip)
        gObjectManip->set_frame(gSceneManip->m_matrix);
}

static void DrawFrame(GLFWwindow *window)
{
    CheckOpenGL(__FILE__, __LINE__);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CheckOpenGL(__FILE__, __LINE__);

    DrawScene();

    CheckOpenGL(__FILE__, __LINE__);
}

int main()
{
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
    InitializeModel();

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
