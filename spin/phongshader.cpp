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

    glLinkProgram(program);
    CheckOpenGL(__FILE__, __LINE__);
    if(!CheckProgramLink(program))
	return 0;

    return program;
}


void PhongShader::ApplyMaterial(const Material& mtl)
{
    glUniform4fv(materialAmbientUniform, 1, mtl.diffuse);
    glUniform4fv(materialDiffuseUniform, 1, mtl.ambient);
    glUniform4fv(materialSpecularUniform, 1, mtl.specular);
    glUniform1f(materialShininessUniform, mtl.shininess);
}

const char *PhongShader::vertexShaderText = "\n\
    uniform mat4 modelview_matrix;\n\
    uniform mat4 modelview_normal_matrix;\n\
    uniform mat4 projection_matrix;\n\
    in vec3 position;\n\
    in vec3 normal;\n\
    \n\
    out vec3 vertex_normal;\n\
    out vec4 vertex_position;\n\
    out vec3 eye_direction;\n\
    \n\
    void main()\n\
    {\n\
    \n\
        vertex_normal = (modelview_normal_matrix * vec4(normal, 0.0)).xyz;\n\
        vertex_position = modelview_matrix * vec4(position, 1.0);\n\
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
    in vec3 vertex_normal;\n\
    in vec4 vertex_position;\n\
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
    \n\
        vec4 light_pos = light_position;\n\
        vec3 ldir = normalize(unitvec(vertex_position, light_pos));\n\
        vec3 refl = reflect(-ldir, normal);\n\
\n\
        vec4 diffuse = max(0, dot(normal, ldir)) * light_color * .8;\n\
        vec4 ambient = light_color * .2;\n\
        vec4 specular = pow(max(0, dot(refl, edir)), material_shininess) * light_color * .8;\n\
    \n\
        color = diffuse * material_diffuse + ambient * material_ambient + specular * material_specular;\n\
    }\n";

void PhongShader::Setup()
{
    program = GenerateProgram(vertexShaderText, fragmentShaderText);
    CheckOpenGL(__FILE__, __LINE__);

    positionAttrib = glGetAttribLocation(program, "position");
    normalAttrib = glGetAttribLocation(program, "normal");
    CheckOpenGL(__FILE__, __LINE__);

    glUseProgram(program);
    CheckOpenGL(__FILE__, __LINE__);

    materialDiffuseUniform = glGetUniformLocation(program, "material_diffuse");
    materialSpecularUniform = glGetUniformLocation(program, "material_specular");
    materialAmbientUniform = glGetUniformLocation(program, "material_ambient");
    materialShininessUniform = glGetUniformLocation(program, "material_shininess");

    lightPositionUniform = glGetUniformLocation(program, "light_position");
    lightColorUniform = glGetUniformLocation(program, "light_color");

    modelviewUniform = glGetUniformLocation(program, "modelview_matrix");
    modelviewNormalUniform = glGetUniformLocation(program, "modelview_normal_matrix");
    projectionUniform = glGetUniformLocation(program, "projection_matrix");
}
