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

#include <vector>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

static void CheckOpenGL(const char *filename, int line)
{
    int glerr;

    if((glerr = glGetError()) != GL_NO_ERROR) {
        printf("GL Error: %04X at %s:%d\n", glerr, filename, line);
    }
}

struct Vertex
{
    float v[3];
    float n[3];
};

extern struct Vertex gVertices[];
extern int gTriangleCount;

struct Material
{
    float diffuse[4];
    float ambient[4];
    float specular[4];
    float shininess;
    Material(const float diffuse_[4], const float ambient_[4],
        const float specular_[4], const float shininess_)
    {
        for(int i = 0; i < 4; i++) diffuse[i] = diffuse_[i];
        for(int i = 0; i < 4; i++) ambient[i] = ambient_[i];
        for(int i = 0; i < 4; i++) specular[i] = specular_[i];
        shininess = shininess_;
    }
    Material()
    {
        diffuse[0] = .8; diffuse[1] = .8; diffuse[2] = .8; diffuse[3] = 1;
        ambient[0] = .2; ambient[1] = .2; ambient[2] = .2; ambient[3] = 1;
        specular[0] = .8; specular[1] = .8; specular[2] = .8; specular[3] = 1;
        shininess = 0;
    }
};

struct DrawList
{
    struct PrimInfo {
        GLenum type;
        GLint start;
        GLsizei count;
        PrimInfo(GLenum t, GLint s, GLsizei c) :
            type(t),
            start(s),
            count(c)
        {}
    };
    GLuint vertexArray;
    bool indexed;
    std::vector<PrimInfo> prims;
    void Draw(bool drawWireframe);
};
