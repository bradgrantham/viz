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
#include <map>
#include <memory>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include "geometry.h"

void CheckOpenGL(const char *filename, int line);

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
    GLenum indexType;
    std::vector<PrimInfo> prims;
    void Draw(bool drawWireframe);
    DrawList() :
        vertexArray(0),
        indexed(false)
    {}
};
typedef std::shared_ptr<DrawList> DrawListPtr;

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
    box bounds;
    DrawListPtr drawList;
    Drawable(const box& b, DrawListPtr dl) :
        bounds(b),
        drawList(dl)
    {}

    virtual void Draw(float objectTime, bool drawWireframe) = 0;
    virtual GLuint GetProgram() = 0;
    virtual EnvironmentUniforms GetEnvironmentUniforms() = 0;
    virtual ~Drawable() {}
};
typedef std::shared_ptr<Drawable> DrawablePtr;

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
    mat4f projection;
    GLuint program;
    EnvironmentUniforms envu; // XXX Clumsy?  Really tied to program, so shouldn't be independent here

    DisplayInfo(const mat4f& modelview_, const mat4f& projection_, GLuint program_, const EnvironmentUniforms& envu_) :
        modelview(modelview_),
        projection(projection_),
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
            for(int i = 0; i < 16; i++) {
                if(d1.projection.m_v[i] < d2.projection.m_v[i])
                    return true;
                if(d1.projection.m_v[i] < d2.projection.m_v[i])
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
typedef std::map<DisplayInfo, std::vector<DrawablePtr>, DisplayInfo::Comparator> DisplayList;

struct Node
{
    box bounds; // Later can cull
    virtual void Visit(const Environment& env, DisplayList& displaylist) = 0;

    Node(const box& bounds_) :
        bounds(bounds_)
    {}
    virtual ~Node() {}
};
typedef std::shared_ptr<Node> NodePtr;

struct Shape : public Node
{
    DrawablePtr drawable;
    virtual void Visit(const Environment& env, DisplayList& displaylist);
    Shape(DrawablePtr& drawable_) :
        Node(drawable_->bounds),
        drawable(drawable_)
    {}
    virtual ~Shape() {}
};
typedef std::shared_ptr<Shape> ShapePtr;

box TransformedBounds(const mat4f& transform, std::vector<NodePtr> children);

struct Group : public Node 
{
    mat4f transform;
    std::vector<NodePtr> children;

    virtual void Visit(const Environment& env, DisplayList& displaylist);

    Group(const mat4f& transform_, std::vector<NodePtr> children_) :
        Node(TransformedBounds(transform_, children_)),
        transform(transform_),
        children(children_)
    {}

    Group(std::vector<NodePtr> children_) :
        Node(TransformedBounds(mat4f::identity, children_)),
        transform(mat4f::identity),
        children(children_)
    {}

    virtual ~Group() {}
};
typedef std::shared_ptr<Group> GroupPtr;


#endif /* _DRAWABLE_H_ */
