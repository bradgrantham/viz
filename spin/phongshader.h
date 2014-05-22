#ifndef _PHONGSHADER_H_
#define _PHONGSHADER_H_

#include "drawable.h"
#include "geometry.h"
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

struct PhongShader
{
    typedef boost::shared_ptr<PhongShader> sptr;

    GLuint modelviewUniform;
    GLuint modelviewNormalUniform;
    GLuint projectionUniform;

    GLuint materialDiffuseUniform;
    GLuint materialAmbientUniform;
    GLuint materialSpecularUniform;
    GLuint materialShininessUniform;

    GLuint lightPositionUniform;
    GLuint lightColorUniform;

    int positionAttrib;
    int normalAttrib; 

    static const char *vertexShaderText;
    static const char *fragmentShaderText;

    GLuint program;

    void Setup();
    void ApplyMaterial(Material::sptr mtl);
};

struct PhongShadedGeometry
{
    typedef boost::shared_ptr<PhongShadedGeometry> sptr;

    DrawList::sptr drawList;
    Material::sptr material;
    PhongShader::sptr phongshader;

    box bounds;

    PhongShadedGeometry(DrawList::sptr dl, Material::sptr mtl, PhongShader::sptr p, const box& b) :
        drawList(dl),
        material(mtl),
        phongshader(p),
        bounds(b)
    {}

    void Draw(float objectTime, bool drawWireframe);
};

#endif /* _PHONGSHADER_H_ */
