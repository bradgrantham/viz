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
