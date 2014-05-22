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

    CheckOpenGL(__FILE__, __LINE__);
}
