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

struct PhongShader : public Shader
{
    typedef boost::shared_ptr<PhongShader> sptr;

    struct Material
    {
        typedef boost::shared_ptr<Material> sptr;
        float diffuse[4];
        float ambient[4];
        float specular[4];
        float shininess;
        Material(const float diffuse_[4], const float ambient_[4],
            const float specular_[4], const float shininess_)
        {
            for(int i = 0; i < 4; i++) diffuse[i] = diffuse_[i];
            for(int i = 0; i < 4; i++) ambient[i] = ambient_[i];
            for(int i = 0; i < 4; i++) specular[i] = specular_[i];
            shininess = shininess_;
        }
        Material()
        {
            diffuse[0] = .8; diffuse[1] = .8; diffuse[2] = .8; diffuse[3] = 1;
            ambient[0] = .2; ambient[1] = .2; ambient[2] = .2; ambient[3] = 1;
            specular[0] = .8; specular[1] = .8; specular[2] = .8; specular[3] = 1;
            shininess = 0;
        }
    };

    struct MaterialUniforms
    {
        GLuint diffuse;
        GLuint ambient;
        GLuint specular;
        GLuint shininess;
    };

    MaterialUniforms mtlu;

    int positionAttrib;
    int normalAttrib; 
    int colorAttrib; 

    static const char *vertexShaderText;
    static const char *fragmentShaderText;

    void ApplyMaterial(Material::sptr mtl);
    virtual void Setup();
    virtual ~PhongShader() {}

    static PhongShader::sptr GetForCurrentContext();
    static PhongShader::sptr gShader;
};

struct PhongShadedGeometry : public Drawable
{
    typedef boost::shared_ptr<PhongShadedGeometry> sptr;

    PhongShader::Material::sptr material;

    PhongShadedGeometry(DrawList::sptr dl, PhongShader::Material::sptr mtl, const box& b) :
        Drawable(b, dl),
        material(mtl)
    {}
    virtual void Draw(float objectTime, bool drawWireframe);
    virtual Shader::sptr GetShader() { return PhongShader::GetForCurrentContext(); }
    virtual ~PhongShadedGeometry() {}
};

#endif /* _PHONGSHADER_H_ */
