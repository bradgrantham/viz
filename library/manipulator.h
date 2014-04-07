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
