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

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <limits>
#include <algorithm>
#include "vectormath.h"

struct box
{
    vec3f m_min;
    vec3f m_max;

    void empty()
    {
        m_min.set(
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max());
        m_max.set(
            -std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max(),
            -std::numeric_limits<float>::max());
    }
    box()
    {
        empty();
    }

    float largest_side() const
    {
        if(m_max[0] - m_min[0] > m_max[1] - m_min[1] &&
            m_max[0] - m_min[0] > m_max[2] - m_min[2])
            return m_max[0] - m_min[0];
        else if(m_max[1] - m_min[1] > m_max[2] - m_min[2])
            return m_max[1] - m_min[1];
        else
            return m_max[2] - m_min[2];
    }

    void extend(float x, float y, float z)
    {
        m_min[0] = std::min(m_min[0], x);
        m_min[1] = std::min(m_min[1], y);
        m_min[2] = std::min(m_min[2], z);
        m_max[0] = std::max(m_max[0], x);
        m_max[1] = std::max(m_max[1], y);
        m_max[2] = std::max(m_max[2], z);
    }

    void extend(const vec3f& v)
    {
        extend(v[0], v[1], v[2]);
    }

    void extend(const box& b)
    {
        extend(b.m_min);
        extend(b.m_max);
    }

    void extend(float x, float y, float z, float r)
    {
        m_min[0] = std::min(m_min[0], x - r);
        m_min[1] = std::min(m_min[1], y - r);
        m_min[2] = std::min(m_min[2], z - r);
        m_max[0] = std::max(m_max[0], x + r);
        m_max[1] = std::max(m_max[1], y + r);
        m_max[2] = std::max(m_max[2], z + r);
    }

};

#endif /* _GEOMETRY_H */
