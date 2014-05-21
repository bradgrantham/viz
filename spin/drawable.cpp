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
