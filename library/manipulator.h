#ifndef _MANIPULATOR_H_
#define _MANIPULATOR_H_

#include "vectormath.h"
#include "geometry.h"

struct manipulator { 
    enum mode {
        ROTATE,	/* rotate in direction of mouse motion */
        ROLL,		/* rotate around Z axis */
        SCROLL,		/* translate in X-Y */
        DOLLY		/* translate in Z */
    } m_mode;

    mat4f m_matrix;

    mat4f m_frame;		/* The coordinate frame for this transform */

    vec3f m_worldX;		/* world X axis in this coordinate space */
    vec3f m_worldY;		/* world Y axis in this coordinate space */	
    vec3f m_worldZ;		/* world Z axis in this coordinate space */

    float m_reference_size;	/* used to calculate translations */
    float m_motion_scale;		/* for dynamic scaling, etc */

    rot4f m_rotation;		/* radians, x, y, z, like OpenGL */
    vec3f m_translation;
    vec3f m_scale;		/* scaled around center. */
    vec3f m_center;		/* ignore by setting to <0,0,0> */

    manipulator();
    manipulator(const box& bounds, float fov);
    void move(float dx, float dy);
    void set_frame(const mat4f& frame);
    void calculate_matrix();
};

#endif /* _MANIPULATOR_H_ */
