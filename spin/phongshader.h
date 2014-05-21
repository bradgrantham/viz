#include "drawable.h"
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

struct PhongShader
{
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
    void ApplyMaterial(const Material& mtl);

};
