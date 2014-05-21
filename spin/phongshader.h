#include "drawable.h"
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

    PhongShadedGeometry(DrawList::sptr dl, Material::sptr mtl, PhongShader::sptr p) :
        drawList(dl),
        material(mtl),
        phongshader(p)
    {}

    void Draw(float objectTime, bool drawWireframe);
};

