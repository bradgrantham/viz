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

struct PhongShader
{
    typedef boost::shared_ptr<PhongShader> sptr;

    struct Material
    {
        typedef boost::shared_ptr<Material> sptr;

        vec4f diffuse;
        GLuint diffuseTexture;
        vec4f ambient;
        vec4f specular;
        float shininess;

        Material(const vec4f& diffuse_, const vec4f& ambient_,
            const vec4f& specular_, float shininess_) :
            diffuse(diffuse_),
            diffuseTexture(GL_NONE),
            ambient(ambient_),
            specular(specular_),
            shininess(shininess_)
        { }

        Material(const vec4f& diffuse_, GLuint diffuseTexture_, const vec4f& ambient_,
            const vec4f& specular_, float shininess_) :
            diffuse(diffuse_),
            diffuseTexture(diffuseTexture_),
            ambient(ambient_),
            specular(specular_),
            shininess(shininess_)
        { }

        Material() :
            diffuse(vec4f(.8, .8, .8, 1)),
            diffuseTexture(GL_NONE),
            ambient(vec4f(.2, .2, .2, 1)),
            specular(vec4f(.8, .8, .8, 1)),
            shininess(0)
        { }
    };

    struct MaterialUniforms
    {
        GLint diffuse;
        GLint ambient;
        GLint specular;
        GLint shininess;
        GLint diffuseTexture; // unused in nontextured 
    };

    struct ProgramVariant {
        GLuint program;
        MaterialUniforms mtlu;
        EnvironmentUniforms envu;

        int positionAttrib;
        int normalAttrib; 
        int colorAttrib; 
        int texcoordAttrib;  // unused in nontextured

        void ApplyMaterial(Material::sptr mtl);
    } nontextured, textured;

    static const char *vertexShaderText;
    static const char *fragmentShaderText;

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
    virtual GLuint GetProgram();
    virtual EnvironmentUniforms GetEnvironmentUniforms();
    virtual ~PhongShadedGeometry() {}
};

#endif /* _PHONGSHADER_H_ */
