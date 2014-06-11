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

#include "drawable.h"

void DrawList::Draw(bool drawWireframe)
{
    glBindVertexArray(vertexArray);
    CheckOpenGL(__FILE__, __LINE__);

    if(indexed) {
        int indexsize = (indexType == GL_UNSIGNED_SHORT) ? 2 : 4; // XXX no BYTE
        unsigned char *baseptr = 0;
        if(drawWireframe) {
            for(size_t i = 0; i < prims.size(); i++) {
                const DrawList::PrimInfo& p = prims[i];
                if(p.type == GL_TRIANGLES)
                    for(int j = 0; j < prims[i].count / 3; j++)
                        glDrawElements(GL_LINE_LOOP, 3, indexType, (const GLvoid*)(baseptr + indexsize * (p.start + j * 3)));
            }
        } else {
            // Leave loop in here instead of elevating because will
            // eventually move to MultiDraw, I think
            for(size_t i = 0; i < prims.size(); i++) {
                const DrawList::PrimInfo& p = prims[i];
                glDrawElements(p.type, p.count, indexType, (const GLvoid*)(baseptr + indexsize * p.start));
            }
        }
    } else {
        if(drawWireframe) {
            for(size_t i = 0; i < prims.size(); i++) {
                const DrawList::PrimInfo& p = prims[i];
                if(p.type == GL_TRIANGLES)
                    for(int j = 0; j < prims[i].count / 3; j++)
                        glDrawArrays(GL_LINE_LOOP, p.start + j * 3, 3);
            }
        } else {
            // Leave loop in here instead of elevating because will
            // eventually move to MultiDraw, I think
            for(size_t i = 0; i < prims.size(); i++) {
                const DrawList::PrimInfo& p = prims[i];
                glDrawArrays(p.type, p.start, p.count);
            }
        }
    }

    CheckOpenGL(__FILE__, __LINE__);
}

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

void Group::Visit(const Environment& env, DisplayList& displaylist)
{
    mat4f newtransform = transform * env.modelview;
    Environment env2(env.projection, newtransform, env.lights);
    for(auto it = children.begin(); it != children.end(); it++)
        (*it)->Visit(env2, displaylist);
}

