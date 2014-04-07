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


#ifndef __VECTORMATH_H__
#define __VECTORMATH_H__

#include <cmath>
#include <cstdio>

template <class V>
inline float vec_dot(const V& v0, const V& v1)
{
    float total = 0;
    for(int i = 0; i < V::dimension(); i++) total += v0[i] * v1[i];
    return total;
}

template <class V>
inline float vec_length(const V& v)
{
    float sum = 0;
    for(int i = 0; i < V::dimension(); i++) sum += v[i] * v[i];
    return sqrtf(sum);
}

template <class V>
inline float vec_length_sq(const V& v)
{
    float sum = 0;
    for(int i = 0; i < V::dimension(); i++) sum += v[i] * v[i];
    return sum;
}

template <class V>
inline V vec_normalize(const V& v)
{
    float l = vec_length(v);
    V tmp;
    for(int i = 0; i < V::dimension(); i++) tmp[i] = v[i] / l;
    return tmp;
}

template <class V>
inline V vec_blend(const V& v0, float w0, const V& v1, float w1)
{
    V tmp;
    for(int i = 0; i < V::dimension(); i++) tmp[i] = v0[i] * w0 + v1[i] * w1;
    return tmp;
}

template <class V>
inline V vec_scale(const V& v0, float w0)
{
    V tmp;
    for(int i = 0; i < V::dimension(); i++) tmp[i] = v0[i] * w0;
    return tmp;
}

/* normalized i, n */
/* doesn't normalize r */
/* r = u - 2 * n * dot(n, u) */
template <class V>
inline V vec_reflect(const V& i, const V& n)
{
    V tmp;
    tmp = vec_blend(i, 1.0f, n, -2.0f * vec_dot(i, n));
    return tmp;
}

// XXX Why aren't these templates?
#define OPS(V) \
    inline bool operator==(const V& v0, const V& v1) \
    { \
        V tmp; \
        for(int i = 0; i < V::dimension(); i++) \
	    if(v0[i] != v1[i]) \
		return false; \
        return true; \
    } \
     \
    inline V operator+(const V& v0, const V& v1) \
    { \
        V tmp; \
        for(int i = 0; i < V::dimension(); i++) tmp[i] = v0[i] + v1[i]; \
        return tmp; \
    } \
     \
    inline V operator-(const V& v0, const V& v1) \
    { \
        V tmp; \
        for(int i = 0; i < V::dimension(); i++) tmp[i] = v0[i] - v1[i]; \
        return tmp; \
    } \
     \
    inline V operator-(const V& v) \
    { \
        V tmp; \
        for(int i = 0; i < V::dimension(); i++) tmp[i] = -v[i]; \
        return tmp; \
    } \
     \
    inline V operator*(float w, const V& v)  \
    { \
        V tmp; \
        for(int i = 0; i < V::dimension(); i++) tmp[i] = v[i] * w; \
        return tmp; \
    } \
     \
    inline V operator/(float w, const V& v) \
    { \
        V tmp; \
        for(int i = 0; i < V::dimension(); i++) tmp[i] = v[i] / w; \
        return tmp; \
    } \
     \
    inline V operator*(const V& v, float w) \
    { \
        V tmp; \
        for(int i = 0; i < V::dimension(); i++) tmp[i] = v[i] * w; \
        return tmp; \
    } \
     \
    inline V operator/(const V& v, float w) \
    { \
        V tmp; \
        for(int i = 0; i < V::dimension(); i++) tmp[i] = v[i] / w; \
        return tmp; \
    } \
     \
    /* Would be nice if we could just define T to be V::comp_type.  Why didn't that work? */ \
    inline V operator*(const V& v0, const V& v1) \
    { \
        V tmp; \
        for(int i = 0; i < V::dimension(); i++) tmp[i] = v0[i] * v1[i]; \
        return tmp; \
    } \
     \

struct vec2f
{
    float m_v[2];
    inline static int dimension() { return 2; }
    typedef float comp_type;

    vec2f(void) { }

    inline vec2f(float x, float y)
        { set(x, y); }

    inline void set(float x, float y)
        { m_v[0] = x; m_v[1] = y; }

    inline vec2f(float v) {
	for(int i = 0; i < 2; i++) m_v[i] = v;
    }

    inline vec2f(const float *v) {
	for(int i = 0; i < 2; i++) m_v[i] = v[i];
    }

    inline vec2f(const vec2f &v) {
	for(int i = 0; i < 2; i++) m_v[i] = v[i];
    }

    inline vec2f &operator=(const float *v) {
	for(int i = 0; i < 2; i++) m_v[i] = v[i];
	return *this;
    }

    inline vec2f &operator=(const vec2f& v) {
	for(int i = 0; i < 2; i++) m_v[i] = v[i];
	return *this;
    }

    inline vec2f(float *v)
        { set(v); }

    inline operator const float*() const { return m_v; }
    inline float& operator[] (int i)
        { return m_v[i]; }

    inline const float& operator[] (int i) const
        { return m_v[i]; }

    inline void clear() { 
	for(int i = 0; i < 2; i++) m_v[i] = 0;
    }

    inline void set(const float *v)
	{ for(int i = 0; i < 2; i++) m_v[i] = v[i]; }

    inline vec2f& normalize() {
	*this = vec_normalize(*this);
	return *this;
    }

    inline vec2f operator*=(float w) {
	for(int i = 0; i < 2; i++) m_v[i] *= w;
	return *this;
    }

    inline vec2f operator/=(float w) {
	for(int i = 0; i < 2; i++) m_v[i] /= w;
	return *this;
    }

    inline vec2f operator+=(const vec2f& v) {
	for(int i = 0; i < 2; i++) m_v[i] += v[i];
	return *this;
    }

    inline vec2f operator-=(const vec2f& v) {
	for(int i = 0; i < 2; i++) m_v[i] -= v[i];
	return *this;
    }
};
OPS(vec2f);

struct vec4f;

struct vec3f
{
    float m_v[3];
    inline static int dimension() { return 3; }
    typedef float comp_type;

    vec3f(void) { }

    inline void set(float x, float y, float z)
        { m_v[0] = x; m_v[1] = y; m_v[2] = z;}

    inline vec3f(float x, float y, float z)
        { set(x, y, z); }

    inline vec3f cross(const vec3f& v1) {
	vec3f tmp;
	tmp[0] = m_v[1] * v1[2] - m_v[2] * v1[1];
	tmp[1] = m_v[2] * v1[0] - m_v[0] * v1[2];
	tmp[2] = m_v[0] * v1[1] - m_v[1] * v1[0];
	*this = tmp;
	return *this;
    }

    inline vec3f(float v) {
	for(int i = 0; i < 3; i++) m_v[i] = v;
    }

    inline vec3f(const float *v) {
	for(int i = 0; i < 3; i++) m_v[i] = v[i];
    }

    inline vec3f(const vec3f &v) {
	for(int i = 0; i < 3; i++) m_v[i] = v[i];
    }

    inline vec3f &operator=(const vec3f& v) {
	for(int i = 0; i < 3; i++) m_v[i] = v[i];
	return *this;
    }

    inline vec3f &operator=(float v) {
	for(int i = 0; i < 3; i++) m_v[i] = v;
	return *this;
    }

    inline vec3f(float *v)
        { set(v); }

    inline float& operator[] (int i)
        { return m_v[i]; }

    inline const float& operator[] (int i) const
        { return m_v[i]; }

    inline operator const float*() const { return m_v; }
    inline operator float*() { return m_v; }

    inline void clear() { 
	for(int i = 0; i < 3; i++) m_v[i] = 0;
    }

    inline void set(const float *v)
	{ for(int i = 0; i < 3; i++) m_v[i] = v[i]; }

    inline float length() const {
	float sum = 0;
	for(int i = 0; i < 3; i++) sum += m_v[i] * m_v[i];
	return (float)sqrtf((double)sum);
    }

    inline vec3f& normalize() {
	*this = vec_normalize(*this);
	return *this;
    }

    inline vec3f operator*=(float w) {
	for(int i = 0; i < 3; i++) m_v[i] *= w;
	return *this;
    }

    inline vec3f operator/=(float w) {
	for(int i = 0; i < 3; i++) m_v[i] /= w;
	return *this;
    }

    inline vec3f operator+=(const vec3f& v) {
	for(int i = 0; i < 3; i++) m_v[i] += v[i];
	return *this;
    }

    inline vec3f operator-=(const vec3f& v) {
	for(int i = 0; i < 3; i++) m_v[i] -= v[i];
	return *this;
    }

    inline vec3f operator*=(const vec3f& v) {
	for(int i = 0; i < 3; i++) m_v[i] *= v[i];
	return *this;
    }

    inline vec3f operator/=(const vec3f& v) {
	for(int i = 0; i < 3; i++) m_v[i] /= v[i];
	return *this;
    }
};
OPS(vec3f);

inline vec3f vec_cross(const vec3f& v0, const vec3f& v1)
{
    vec3f tmp;
    tmp[0] = v0[1] * v1[2] - v0[2] * v1[1];
    tmp[1] = v0[2] * v1[0] - v0[0] * v1[2];
    tmp[2] = v0[0] * v1[1] - v0[1] * v1[0];
    return tmp;
}

struct vec4f
{
    float m_v[4];
    inline static int dimension() { return 4; }
    typedef float comp_type;

    vec4f(void) { }

    inline void set(float x, float y, float z, float w)
        { m_v[0] = x; m_v[1] = y; m_v[2] = z; m_v[3] = w;}

    inline vec4f(float x, float y, float z, float w)
        { set(x, y, z, w); }

    inline vec4f(float v) {
	for(int i = 0; i < 4; i++) m_v[i] = v;
    }

    inline vec4f(const float *v) {
	for(int i = 0; i < 4; i++) m_v[i] = v[i];
    }

    inline vec4f(const vec4f &v) {
	for(int i = 0; i < 4; i++) m_v[i] = v[i];
    }

    inline vec4f &operator=(const vec4f& v) {
	for(int i = 0; i < 4; i++) m_v[i] = v[i];
	return *this;
    }

    inline vec4f(float *v)
        { set(v); }

    inline float& operator[] (int i)
        { return m_v[i]; }

    inline operator const float*() const { return m_v; }

    inline const float& operator[] (int i) const
        { return m_v[i]; }

    inline void clear() { 
	for(int i = 0; i < 4; i++) m_v[i] = 0;
    }

    inline void set(const float *v)
	{ for(int i = 0; i < 4; i++) m_v[i] = v[i]; }

    inline float length() const {
	float sum = 0;
	for(int i = 0; i < 4; i++) sum += m_v[i] * m_v[i];
	return (float)sqrtf((double)sum);
    }

    inline vec4f& normalize() {
	*this = vec_normalize(*this);
	return *this;
    }

    inline vec4f operator*=(float w) {
	for(int i = 0; i < 4; i++) m_v[i] *= w;
	return *this;
    }

    inline vec4f operator/=(float w) {
	for(int i = 0; i < 4; i++) m_v[i] /= w;
	return *this;
    }

    inline vec4f operator+=(const vec4f& v) {
	for(int i = 0; i < 4; i++) m_v[i] += v[i];
	return *this;
    }

    inline vec4f operator-=(const vec4f& v) {
	for(int i = 0; i < 4; i++) m_v[i] -= v[i];
	return *this;
    }
};
OPS(vec4f);

//
// With your left hand up, fingers up, palm facing away, thumb facing to
// the right, thumb is v0-v1, index finger is v0-v2 : plane normal
// sticks out the back of your the hand towards you.
//
inline vec4f make_plane(const vec3f& v0, const vec3f& v1, const vec3f& v2)
{
    vec3f xaxis, yaxis;
    vec3f plane;

    xaxis = vec_blend(v1, 1.0, v0, -1.0);

    yaxis = vec_blend(v2, 1.0, v0, -1.0);

    plane = vec_cross(xaxis, yaxis);
    plane.normalize();

    float D = vec_dot(-v0, plane);
    return vec4f(plane[0], plane[1], plane[2], D);
}

struct rot4f : public vec4f
{
    rot4f() :
        vec4f(0.0f, 1.0f, 0.0f, 0.0f)
    {
    }
    rot4f(float angle, float x, float y, float z) :
        vec4f(angle, x, y, z)
    {
    }
    inline void set_axis(float x, float y, float z) {
	m_v[1] = x;
	m_v[2] = y;
	m_v[3] = z;
    }

    inline void set_axis(vec3f &axis) {
	m_v[1] = axis[0];
	m_v[2] = axis[1];
	m_v[3] = axis[2];
    }

    rot4f& mult(const rot4f& m1, const rot4f &m2);
};

rot4f operator*(const rot4f& r1, const rot4f& r2);


struct mat4f
{
    float m_v[16];
    inline static int dimension() { return 16; }
    typedef float comp_type;
    static mat4f identity;

    mat4f() { }

    mat4f(float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33) {

	m_v[0] = m00; m_v[1] = m01; m_v[2] = m02; m_v[3] = m03;
	m_v[4] = m10; m_v[5] = m11; m_v[6] = m12; m_v[7] = m13;
	m_v[8] = m20; m_v[9] = m21; m_v[10] = m22; m_v[11] = m23;
	m_v[12] = m30; m_v[13] = m31; m_v[14] = m32; m_v[15] = m33;
    }

    // mat4f::mult_nm does not perform inverse transpose - just multiplies with
    //     v[3] = 0
    inline vec3f mult_nm(vec3f &in) {
	int i;
	vec4f t;

	for(i = 0; i < 4; i++)
	    t[i] =
		m_v[0 + i] * in[0] + 
		m_v[4 + i] * in[1] + 
		m_v[8 + i] * in[2];

	t[0] /= t[3];
	t[1] /= t[3];
	t[2] /= t[3];
	return vec3f(t[0], t[1], t[2]);
    }

    inline mat4f& transpose(mat4f& in) {
	mat4f t;
	int i, j;

	t = in;
	for(i = 0; i < 4; i++)
	    for(j = 0; j < 4; j++) 
		m_v[i + j * 4] = t[j + i * 4];

	return *this;
    }
    mat4f& transpose() { return transpose(*this); }

    inline float determinant() const {
	return (m_v[0] * m_v[5] - m_v[1] * m_v[4]) *
	    (m_v[10] * m_v[15] - m_v[11] * m_v[14]) + 
	    (m_v[2] * m_v[4] - m_v[0] * m_v[6]) *
	    (m_v[9] * m_v[15] - m_v[11] * m_v[13]) + 
	    (m_v[0] * m_v[7] - m_v[3] * m_v[4]) *
	    (m_v[9] * m_v[14] - m_v[10] * m_v[13]) + 
	    (m_v[1] * m_v[6] - m_v[2] * m_v[5]) *
	    (m_v[8] * m_v[15] - m_v[11] * m_v[12]) + 
	    (m_v[3] * m_v[5] - m_v[1] * m_v[7]) *
	    (m_v[8] * m_v[14] - m_v[10] * m_v[12]) + 
	    (m_v[2] * m_v[7] - m_v[3] * m_v[6]) *
	    (m_v[8] * m_v[13] - m_v[9] * m_v[12]);
    }

    bool invert(const mat4f& in, bool singular_fail = true);
    bool invert() { return invert(*this); }

    static inline mat4f translation(float x, float y, float z) {
	mat4f m(identity);
	m[12] = x;
	m[13] = y;
	m[14] = z;
	return m;
    }

    static inline mat4f scale(float x, float y, float z) {
	mat4f m(identity);
	m[0] = x;
	m[5] = y;
	m[10] = z;

	return m;
    }

    static inline mat4f frustum(float left, float right, float bottom, float top, float nearClip, float farClip)
    {
        mat4f m(identity);

        float A = (right + left) / (right - left);
        float B = (top + bottom) / (top - bottom);
        float C = - (farClip + nearClip) / (farClip - nearClip);
        float D = - 2 * farClip * nearClip / (farClip - nearClip);

        m[0] = 2 * nearClip / (right - left);
        m[5] = 2 * nearClip / (top - bottom);

        m[2] = A;
        m[6] = B;
        m[10] = C;
        m[14] = D;

        m[11] = -1;
        m[15] = 0;

        return m;
    }

    static inline mat4f rotation(float a, float x, float y, float z) {
	mat4f m;
	float c, s, t;

	c = (float)cos(a);
	s = (float)sin(a);
	t = 1.0f - c;

	m[0] = t * x * x + c;
	m[1] = t * x * y + s * z;
	m[2] = t * x * z - s * y;
	m[3] = 0;

	m[4] = t * x * y - s * z;
	m[5] = t * y * y + c;
	m[6] = t * y * z + s * x;
	m[7] = 0;

	m[8] = t * x * z + s * y;
	m[9] = t * y * z - s * x;
	m[10] = t * z * z + c;
	m[11] = 0;

	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;

	return m;
    }

    inline mat4f(const rot4f& r) {
	(*this) = rotation(r[0], r[1], r[2], r[3]);
    }

    void calc_rot4f(rot4f *out) const;

    inline mat4f& mult(mat4f& m1, mat4f &m2) {
	mat4f t;
	int i, j;

	for(j = 0; j < 4; j++)
	    for(i = 0; i < 4; i++)
	       t[i * 4 + j] = m1[i * 4 + 0] * m2[0 * 4 + j] +
		   m1[i * 4 + 1] * m2[1 * 4 + j] +
		   m1[i * 4 + 2] * m2[2 * 4 + j] +
		   m1[i * 4 + 3] * m2[3 * 4 + j];

	*this = t;
	return *this;
    }

    inline mat4f(const float *v) {
	for(int i = 0; i < 16; i++) m_v[i] = v[i];
    }

    inline mat4f(const mat4f &v) {
	for(int i = 0; i < 16; i++) m_v[i] = v[i];
    }

    inline mat4f &operator=(const mat4f& v) {
	for(int i = 0; i < 16; i++) m_v[i] = v[i];
	return *this;
    }

    inline mat4f(float *v)
        { set(v); }

    inline float& operator[] (int i)
        { return m_v[i]; }

    inline const float& operator[] (int i) const
        { return m_v[i]; }

    inline void clear() { 
	for(int i = 0; i < 16; i++) m_v[i] = 0;
    }

    inline void set(const float *v)
	{ for(int i = 0; i < 16; i++) m_v[i] = v[i]; }

    inline void store(float *v)
	{ for(int i = 0; i < 16; i++) v[i] = m_v[i]; }
};

inline mat4f operator*(const mat4f& m1, const mat4f& m2)
{
    mat4f t;
    int i, j;

    for(j = 0; j < 4; j++)
	for(i = 0; i < 4; i++)
	   t[i * 4 + j] =
	       m1[i * 4 + 0] * m2[0 * 4 + j] +
	       m1[i * 4 + 1] * m2[1 * 4 + j] +
	       m1[i * 4 + 2] * m2[2 * 4 + j] +
	       m1[i * 4 + 3] * m2[3 * 4 + j];

    return t;
}

inline vec4f operator*(const vec4f& in, const mat4f& m)
{
    int i;
    vec4f t;

    for(i = 0; i < 4; i++)
	t[i] =
	    m[0 + i] * in[0] + 
	    m[4 + i] * in[1] + 
	    m[8 + i] * in[2] + 
	    m[12 + i] * in[3];
    return t;
}

inline vec3f operator*(const vec3f& in, const mat4f& m)
{
    int i;
    vec4f t;

    for(i = 0; i < 4; i++)
	t[i] =
	    m[0 + i] * in[0] + 
	    m[4 + i] * in[1] + 
	    m[8 + i] * in[2] + 
	    m[12 + i];
    return vec3f(t.m_v);
}

// XXX There's ray code in the original projects/modules/singles/linmath.h

#endif /* __VECTORMATH_H__ */

/*!
 * vi:tabstop=8
 !*/
