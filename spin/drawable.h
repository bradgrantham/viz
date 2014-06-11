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

#ifndef _DRAWABLE_H_
#define _DRAWABLE_H_

#include <vector>

#include <memory>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include "geometry.h"

static void CheckOpenGL(const char *filename, int line)
{
    int glerr;

    if((glerr = glGetError()) != GL_NO_ERROR) {
        printf("GL Error: %04X at %s:%d\n", glerr, filename, line);
    }
}

struct DrawList
{
    typedef std::shared_ptr<DrawList> sptr;
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
    GLenum indexType;
    std::vector<PrimInfo> prims;
    void Draw(bool drawWireframe);
    DrawList() :
        vertexArray(0),
        indexed(false)
    {}
};

struct EnvironmentUniforms
{
    GLuint modelview;
    GLuint modelviewNormal;
    GLuint projection;

    GLuint lightPosition;
    GLuint lightColor;
};

struct Drawable
{
    typedef std::shared_ptr<Drawable> sptr;
    box bounds;
    DrawList::sptr drawList;
    Drawable(const box& b, DrawList::sptr dl) :
        bounds(b),
        drawList(dl)
    {}

    virtual void Draw(float objectTime, bool drawWireframe) = 0;
    virtual GLuint GetProgram() = 0;
    virtual EnvironmentUniforms GetEnvironmentUniforms() = 0;
    virtual ~Drawable() {}
};

#endif /* _DRAWABLE_H_ */
