/*
 * $Header: /home/grantham/cvsroot/projects/modules/singles/linmath.cpp,v 1.1 2007/01/18 11:37:32 grantham Exp $
 */

#include <cmath>
#include "vectormath.h"

mat4f mat4f::identity(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
);

#define EPSILON .00001

bool mat4f::invert(const mat4f& mat, bool singular_fail)
{
    int		i, rswap;
    float	det, div, swap;
    mat4f	hold;

    hold = mat;
    *this = identity;
    det = mat.determinant();
    if(singular_fail && (fabs(det) < EPSILON)) /* singular? */
	return false;

    rswap = 0;
    /* this loop isn't entered unless [0 + 0] > EPSILON and det > EPSILON,
	 so rswap wouldn't be 0, but I initialize so as not to get warned */
    if(fabs(hold[0]) < EPSILON)
    {
        if(fabs(hold[1]) > EPSILON)
            rswap = 1;
        else if(fabs(hold[2]) > EPSILON)
	    rswap = 2;
        else if(fabs(hold[3]) > EPSILON)
	    rswap = 3;

        for(i = 0; i < 4; i++)
	{
            swap = hold[i * 4 + 0];
            hold[i * 4 + 0] = hold[i * 4 + rswap];
            hold[i * 4 + rswap] = swap;

            swap = m_v[i * 4 + 0];
            m_v[i * 4 + 0] = m_v[i * 4 + rswap];
            m_v[i * 4 + rswap] = swap;
        }
    }
        
    div = hold[0];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 0] /= div;
        m_v[i * 4 + 0] /= div;
    }

    div = hold[1];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 1] -= div * hold[i * 4 + 0];
        m_v[i * 4 + 1] -= div * m_v[i * 4 + 0];
    }
    div = hold[2];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 2] -= div * hold[i * 4 + 0];
        m_v[i * 4 + 2] -= div * m_v[i * 4 + 0];
    }
    div = hold[3];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 3] -= div * hold[i * 4 + 0];
        m_v[i * 4 + 3] -= div * m_v[i * 4 + 0];
    }

    if(fabs(hold[5]) < EPSILON){
        if(fabs(hold[6]) > EPSILON)
	    rswap = 2;
        else if(fabs(hold[7]) > EPSILON)
	    rswap = 3;

        for(i = 0; i < 4; i++)
	{
            swap = hold[i * 4 + 1];
            hold[i * 4 + 1] = hold[i * 4 + rswap];
            hold[i * 4 + rswap] = swap;

            swap = m_v[i * 4 + 1];
            m_v[i * 4 + 1] = m_v[i * 4 + rswap];
            m_v[i * 4 + rswap] = swap;
        }
    }

    div = hold[5];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 1] /= div;
        m_v[i * 4 + 1] /= div;
    }

    div = hold[4];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 0] -= div * hold[i * 4 + 1];
        m_v[i * 4 + 0] -= div * m_v[i * 4 + 1];
    }
    div = hold[6];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 2] -= div * hold[i * 4 + 1];
        m_v[i * 4 + 2] -= div * m_v[i * 4 + 1];
    }
    div = hold[7];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 3] -= div * hold[i * 4 + 1];
        m_v[i * 4 + 3] -= div * m_v[i * 4 + 1];
    }

    if(fabs(hold[10]) < EPSILON){
        for(i = 0; i < 4; i++)
	{
            swap = hold[i * 4 + 2];
            hold[i * 4 + 2] = hold[i * 4 + 3];
            hold[i * 4 + 3] = swap;

            swap = m_v[i * 4 + 2];
            m_v[i * 4 + 2] = m_v[i * 4 + 3];
            m_v[i * 4 + 3] = swap;
        }
    }

    div = hold[10];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 2] /= div;
        m_v[i * 4 + 2] /= div;
    }

    div = hold[8];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 0] -= div * hold[i * 4 + 2];
        m_v[i * 4 + 0] -= div * m_v[i * 4 + 2];
    }
    div = hold[9];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 1] -= div * hold[i * 4 + 2];
        m_v[i * 4 + 1] -= div * m_v[i * 4 + 2];
    }
    div = hold[11];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 3] -= div * hold[i * 4 + 2];
        m_v[i * 4 + 3] -= div * m_v[i * 4 + 2];
    }

    div = hold[15];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 3] /= div;
        m_v[i * 4 + 3] /= div;
    }

    div = hold[12];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 0] -= div * hold[i * 4 + 3];
        m_v[i * 4 + 0] -= div * m_v[i * 4 + 3];
    }
    div = hold[13];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 1] -= div * hold[i * 4 + 3];
        m_v[i * 4 + 1] -= div * m_v[i * 4 + 3];
    }
    div = hold[14];
    for(i = 0; i < 4; i++)
    {
        hold[i * 4 + 2] -= div * hold[i * 4 + 3];
        m_v[i * 4 + 2] -= div * m_v[i * 4 + 3];
    }
    
    return true;
}

void mat4f::calc_rot4f(rot4f *rotation) const
{
    float cosine;
    float sine;
    float d;

    cosine = (m_v[0] + m_v[5] + m_v[10] - 1.0f) / 2.0f;

    /* grantham 20000418 - I know this fixes the mysterious */
    /* NAN matrices, but I have no idea what the above number is supposed */
    /* to do, so I don't know why this happens */
    if(cosine > 1.0){
#if defined(DEBUG)
	fprintf(stderr, "XXX acos of greater than 1! (clamped)\n");
#endif // DEBUG
	cosine = 1.0;
    }
    if(cosine < -1.0){
#if defined(DEBUG)
	fprintf(stderr, "XXX acos of less than -1! (clamped)\n");
#endif // DEBUG
	cosine = -1.0;
    }

    (*rotation)[0] = (float)acos(cosine);

#if defined(DEBUG)
    if((*rotation)[0] != (*rotation)[0]) /* isNAN */
	abort();
#endif // DEBUG

    sine = (float)sin((*rotation)[0]);

    (*rotation)[1] = (m_v[6] - m_v[9]);
    (*rotation)[2] = (m_v[8] - m_v[2]);
    (*rotation)[3] = (m_v[1] - m_v[4]);

    d = sqrt((*rotation)[1] * (*rotation)[1] +
	(*rotation)[2] * (*rotation)[2] +
	(*rotation)[3] * (*rotation)[3]);

    (*rotation)[1] /= d;
    (*rotation)[2] /= d;
    (*rotation)[3] /= d;
}

rot4f &rot4f::mult(const rot4f &rotation1, const rot4f &rotation2)
{
    mat4f matrix1(rotation1);
    mat4f matrix2(rotation2);
    mat4f matrix3;
    float dist;

    matrix3 = matrix1 * matrix2;
    matrix3.calc_rot4f(this);

    dist = (float)sqrt(m_v[1] * m_v[1] + m_v[2] * m_v[2] +
        m_v[3] * m_v[3]);

#if defined(DEBUG)
    if(m_v[0] != m_v[0]) /* isNAN */
	abort();
#endif // DEBUG

    m_v[1] /= dist;
    m_v[2] /= dist;
    m_v[3] /= dist;

    return *this;
}

rot4f operator*(const rot4f& r1, const rot4f& r2)
{
    rot4f t;

    t.mult(r1, r2);
    return t;
}

#ifdef LINMATH_TEST

#include <stdio.h>

int main(int argc, char **argv)
{
    vec3f xaxis(1, 0, 0);
    vec3f yaxis(0, 1, 0);
    vec3f zaxis;
    vec3f foobar(1, 1, 1);

    printf("length of foobar is %f\n", foobar.length());
    printf("length of foobar is %f\n", float(foobar));

    zaxis = xaxis.cross(yaxis);
    printf("x cross y is <%f, %f, %f>\n", zaxis[0], zaxis[1], zaxis[2]);
}

#endif

/*!
 * vi:sw=4 ts=8
 !*/
