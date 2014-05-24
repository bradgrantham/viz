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

#include <string>
#include "phongshader.h"

// XXX this option and the compiler utility functions below maybe should be
// in a separate .cpp...
bool gPrintShaderLog = true;

static bool CheckShaderCompile(GLuint shader, const std::string& shader_name)
{
    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_TRUE)
	return true;

    if(gPrintShaderLog) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        if (length > 0) {
            char log[length];
            glGetShaderInfoLog(shader, length, NULL, log);
            fprintf(stderr, "%s shader error log:\n%s\n", shader_name.c_str(), log);
        }

        fprintf(stderr, "%s compile failure.\n", shader_name.c_str());
        fprintf(stderr, "shader text:\n");
        glGetShaderiv(shader, GL_SHADER_SOURCE_LENGTH, &length);
        char source[length];
        glGetShaderSource(shader, length, NULL, source);
        fprintf(stderr, "%s\n", source);
    }
    return false;
}

static bool CheckProgramLink(GLuint program)
{
    int status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status == GL_TRUE)
	return true;

    if(gPrintShaderLog) {
        int log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

        if (log_length > 0) {
            char log[log_length];
            glGetProgramInfoLog(program, log_length, NULL, log);
            fprintf(stderr, "program error log: %s\n",log);
        }
    }

    return false;
}


static GLuint GenerateProgram(const std::string& vertex_shader_text, const std::string& fragment_shader_text)
{
    CheckOpenGL(__FILE__, __LINE__);
    std::string spec_string;

    spec_string = "#version 140\n";

    // reset line number so that I can view errors with the line number
    // they have in the base shaders.
    spec_string += "#line 0\n";

    std::string vertex_shader_string = spec_string + vertex_shader_text;
    std::string fragment_shader_string = spec_string + fragment_shader_text;

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const char *string = vertex_shader_string.c_str();
    glShaderSource(vertex_shader, 1, &string, NULL);
    glCompileShader(vertex_shader);
    if(!CheckShaderCompile(vertex_shader, "vertex shader"))
	return 0;

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    string = fragment_shader_string.c_str();
    glShaderSource(fragment_shader, 1, &string, NULL);
    glCompileShader(fragment_shader);
    if(!CheckShaderCompile(fragment_shader, "fragment shader"))
	return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    CheckOpenGL(__FILE__, __LINE__);

    glLinkProgram(program);
    CheckOpenGL(__FILE__, __LINE__);
    if(!CheckProgramLink(program))
	return 0;

    return program;
}

mvoid PhongShader::ProgramVariant::ApplyMaterial(PhongShader::Material::sptr mtl)
{
    glUseProgram(program); // can switch to tex here
    glUniform4fv(mtlu.ambient, 1, mtl->ambient);
    glUniform4fv(mtlu.diffuse, 1, mtl->diffuse);
    glUniform4fv(mtlu.specular, 1, mtl->specular);
    glUniform1f(mtlu.shininess, mtl->shininess);
    if(mtlu.diffuseTexture != -1)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mtl->diffuseTexture);
        glUniform1i(mtlu.diffuseTexture, 0);
    }
    CheckOpenGL(__FILE__, __LINE__);
}

const char *PhongShader::vertexShaderText = "\n\
    uniform mat4 modelview_matrix;\n\
    uniform mat4 modelview_normal_matrix;\n\
    uniform mat4 projection_matrix;\n\
    in vec3 position;\n\
    in vec3 normal;\n\
    in vec4 color;\n\
    #if defined(TEXTURING)\n\
    in vec2 texcoord;\n\
    #endif\n\
    \n\
    out vec3 vertex_normal;\n\
    out vec4 vertex_position;\n\
    out vec4 vertex_color;\n\
    out vec3 eye_direction;\n\
    #if defined(TEXTURING)\n\
    out vec2 vertex_texcoord;\n\
    #endif\n\
    \n\
    void main()\n\
    {\n\
    \n\
        vertex_normal = (modelview_normal_matrix * vec4(normal, 0.0)).xyz;\n\
        vertex_position = modelview_matrix * vec4(position, 1.0);\n\
        vertex_color = color;\n\
        #if defined(TEXTURING)\n\
        vertex_texcoord = texcoord;\n\
        #endif\n\
        eye_direction = -vertex_position.xyz;\n\
    \n\
        gl_Position = projection_matrix * modelview_matrix * vec4(position, 1.0);\n\
        ;\n\
    }\n";

const char *PhongShader::fragmentShaderText = "\n\
    uniform vec4 material_diffuse;\n\
    uniform vec4 material_specular;\n\
    uniform vec4 material_ambient;\n\
    uniform float material_shininess;\n\
    \n\
    uniform vec4 light_position;\n\
    uniform vec4 light_color;\n\
    \n\
    #if defined(TEXTURING)\n\
    uniform sampler2D material_diffuse_texture;\n\
    #endif\n\
    \n\
    in vec3 vertex_normal;\n\
    in vec4 vertex_position;\n\
    in vec4 vertex_color;\n\
    #if defined(TEXTURING)\n\
    in vec2 vertex_texcoord;\n\
    #endif\n\
    in vec3 eye_direction;\n\
    out vec4 color;\n\
    \n\
    vec3 unitvec(vec4 p1, vec4 p2)\n\
    {\n\
        if(p1.w == 0 && p2.w == 0)\n\
            return vec3(p2 - p1);\n\
        if(p1.w == 0)\n\
            return vec3(-p1);\n\
        if(p2.w == 0)\n\
            return vec3(p2);\n\
        return p2.xyz / p2.w - p1.xyz / p1.w;\n\
    }\n\
    \n\
    void main()\n\
    {\n\
        vec3 normal = normalize(vertex_normal);\n\
    \n\
        int light;\n\
        vec3 edir = normalize(eye_direction);\n\
        if(dot(normal, edir) < 0)\n\
            normal *= -1;\n\
    \n\
        vec4 light_pos = light_position;\n\
        vec3 ldir = normalize(unitvec(vertex_position, light_pos));\n\
        vec3 refl = reflect(-ldir, normal);\n\
\n\
        #if defined(TEXTURING)\n\
        vec4 diffuse = max(0, dot(normal, ldir)) * light_color * texture(material_diffuse_texture, vertex_texcoord); ;\n\
        #else\n\
        vec4 diffuse = max(0, dot(normal, ldir)) * light_color;\n\
        #endif\n\
        vec4 ambient = light_color;\n\
        vec4 specular = pow(max(0, dot(refl, edir)), material_shininess) * light_color * .8;\n\
    \n\
        color = diffuse * material_diffuse * vertex_color + ambient * material_ambient * vertex_color + specular * material_specular;\n\
    }\n";

void SetupVariant(bool texturing, PhongShader::ProgramVariant& v)
{
    std::string preamble = texturing ? "#define TEXTURING\n" : "#undef TEXTURING\n";
    v.program = GenerateProgram(preamble + PhongShader::vertexShaderText, preamble + PhongShader::fragmentShaderText);
    CheckOpenGL(__FILE__, __LINE__);

    glUseProgram(v.program);

    v.positionAttrib = glGetAttribLocation(v.program, "position");
    v.normalAttrib = glGetAttribLocation(v.program, "normal");
    v.colorAttrib = glGetAttribLocation(v.program, "color");
    if(texturing) 
        v.texcoordAttrib = glGetAttribLocation(v.program, "texcoord");
    CheckOpenGL(__FILE__, __LINE__);

    v.mtlu.diffuse = glGetUniformLocation(v.program, "material_diffuse");
    v.mtlu.diffuseTexture = texturing ? glGetUniformLocation(v.program, "material_diffuse_texture") : -1;
    v.mtlu.specular = glGetUniformLocation(v.program, "material_specular");
    v.mtlu.ambient = glGetUniformLocation(v.program, "material_ambient");
    v.mtlu.shininess = glGetUniformLocation(v.program, "material_shininess");

    v.envu.lightPosition = glGetUniformLocation(v.program, "light_position");
    v.envu.lightColor = glGetUniformLocation(v.program, "light_color");

    v.envu.modelview = glGetUniformLocation(v.program, "modelview_matrix");
    v.envu.modelviewNormal = glGetUniformLocation(v.program, "modelview_normal_matrix");
    v.envu.projection = glGetUniformLocation(v.program, "projection_matrix");
    CheckOpenGL(__FILE__, __LINE__);
}

void PhongShader::Setup()
{
    SetupVariant(false, nontextured);
    SetupVariant(true, textured);
}

PhongShader::sptr PhongShader::gShader;

PhongShader::sptr PhongShader::GetForCurrentContext()
{
    // XXX only handle one context that doesn't change for now
    if(!gShader) {
        gShader = PhongShader::sptr(new PhongShader());
        gShader->Setup();
    }
    return gShader;
}

GLuint PhongShadedGeometry::GetProgram()
{
    if(material->diffuseTexture != GL_NONE)
        return PhongShader::GetForCurrentContext()->textured.program;
    else
        return PhongShader::GetForCurrentContext()->nontextured.program;
}

EnvironmentUniforms PhongShadedGeometry::GetEnvironmentUniforms()
{
    if(material->diffuseTexture != GL_NONE)
        return PhongShader::GetForCurrentContext()->textured.envu;
    else
        return PhongShader::GetForCurrentContext()->nontextured.envu;
}

void PhongShadedGeometry::Draw(float objectTime, bool drawWireframe)
{
    CheckOpenGL(__FILE__, __LINE__);

    if(material->diffuseTexture != GL_NONE)
        PhongShader::GetForCurrentContext()->textured.ApplyMaterial(material);
    else
        PhongShader::GetForCurrentContext()->nontextured.ApplyMaterial(material);
    CheckOpenGL(__FILE__, __LINE__);

    drawList->Draw(drawWireframe);
}
