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
#include <map>
#include <libgen.h>
#include <FreeImagePlus.h>
#include "trisrc_loader.h"
#include "phongshader.h"

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

namespace TriSrcLoader
{

void LoadCheckerBoard(int w, int h, int checkw, int checkh)
{
    unsigned char image[w * h * 3];

    for(int j = 0; j < h; j++)
        for(int i = 0; i < w; i++) {
            int value = (((w / checkw) + (h / checkh)) % 2 == 0) ? 255 : 0;
            image[(j * checkw + i) * 3 + 0] = value;
            image[(j * checkw + i) * 3 + 1] = value;
            image[(j * checkw + i) * 3 + 2] = value;
        }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    CheckOpenGL(__FILE__, __LINE__);
}

GLuint LoadTexture(const std::string& filename)
{
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    CheckOpenGL(__FILE__, __LINE__);

    fipImage image;
    bool success;

    if (!(success = image.load(filename.c_str()))) {

        std::cerr << "LoadTexture: Failed to load image from " <<
            filename << std::endl;

    } else if (!(success = image.convertTo32Bits())) {

        std::cerr << "LoadTexture: Couldn't convert image to 24 bits " <<
            filename << std::endl;

    } else {

        GLenum sourceFormat;
        GLenum sourceType;
        bool handled = false;

        if (image.getImageType() == FIT_RGBF) {

            sourceFormat = GL_RGB;
            sourceType = GL_FLOAT;
            handled = true;

        } else if (image.getImageType() == FIT_BITMAP){

            unsigned int redMask = FreeImage_GetRedMask(image);
            sourceFormat = (redMask == 0x00FF0000)? GL_BGRA : GL_RGBA;
            sourceType = GL_UNSIGNED_BYTE;
            handled = true;

        }

        if(!handled) {
            std::cerr << "Unhandled FIP image type: " << image.getImageType() << std::endl;
            success = false;

        } else {

            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.getWidth(), image.getHeight(), 0, sourceFormat, sourceType, image.accessPixels());
            glGenerateMipmap(GL_TEXTURE_2D);
    CheckOpenGL(__FILE__, __LINE__);
        }

    }

    if (!success) {
        std::cerr << "GL Renderer: loading checkerboard instead." << std::endl;
        LoadCheckerBoard(64, 64, 4, 4);
    }

    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    CheckOpenGL(__FILE__, __LINE__);

    return texture;
}

struct Vertex
{
    float v[3];
    float n[3];
    float c[4];
    float t[2];
};

struct VertexComparator
{
    bool operator()(const Vertex& v1, const Vertex& v2) const
    {
        for(int i = 0; i < 3; i++)
            if(v1.v[i] < v2.v[i]) {
                return true;
            } else if(v1.v[i] > v2.v[i]) {
                return false;
            }
        for(int i = 0; i < 3; i++)
            if(v1.n[i] < v2.n[i]) {
                return true;
            } else if(v1.n[i] > v2.n[i]) {
                return false;
            }
        for(int i = 0; i < 4; i++)
            if(v1.c[i] < v2.c[i]) {
                return true;
            } else if(v1.c[i] > v2.c[i]) {
                return false;
            }
        for(int i = 0; i < 2; i++)
            if(v1.t[i] < v2.t[i]) {
                return true;
            } else if(v1.t[i] > v2.t[i]) {
                return false;
            }
        return false;
    }
};


PhongShadedGeometry::sptr MakeShape(PhongShader::Material::sptr mtl, Vertex *vertices, size_t vertexCount, unsigned int *indices, int indexCount)
{
    PhongShader::sptr shader = PhongShader::GetForCurrentContext();

    DrawList::sptr drawlist(new DrawList);
    glGenVertexArrays(1, &drawlist->vertexArray);
    glBindVertexArray(drawlist->vertexArray);
    drawlist->indexed = false;
    drawlist->prims.push_back(DrawList::PrimInfo(GL_TRIANGLES, 0, indexCount));
    CheckOpenGL(__FILE__, __LINE__);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexCount, indices, GL_STATIC_DRAW);
    CheckOpenGL(__FILE__, __LINE__);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexCount, vertices, GL_STATIC_DRAW);
    CheckOpenGL(__FILE__, __LINE__);

    size_t coordSize = sizeof(float) * 3;
    size_t normalSize = sizeof(float) * 3;
    size_t colorSize = sizeof(float) * 4;
    size_t texcoordSize = sizeof(float) * 2;

    size_t stride = coordSize + normalSize + colorSize + texcoordSize;
    size_t normalOffset = coordSize;
    size_t colorOffset = normalOffset + normalSize;
    // size_t texcoordOffset = colorOffset + colorSize;

    glVertexAttribPointer(shader->positionAttrib, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(shader->positionAttrib);

    glVertexAttribPointer(shader->normalAttrib, 3, GL_FLOAT, GL_FALSE, stride, (void*)normalOffset);
    glEnableVertexAttribArray(shader->normalAttrib);

    glVertexAttribPointer(shader->colorAttrib, 3, GL_FLOAT, GL_FALSE, stride, (void*)colorOffset);
    glEnableVertexAttribArray(shader->colorAttrib);

    // glVertexAttribPointer(shader->texcoordAttrib, 3, GL_FLOAT, GL_FALSE, stride, (void*)colorOffset);
    // glEnableVertexAttribArray(shader->texcoordAttrib);
    CheckOpenGL(__FILE__, __LINE__);

    glBindVertexArray(GL_NONE);

    box bounds;
    for(int i = 0; i < vertexCount; i++)
        bounds.extend(vertices[i].v[0], vertices[i].v[1], vertices[i].v[2]);

    return PhongShadedGeometry::sptr(new PhongShadedGeometry(drawlist, mtl, bounds));
}

struct indexed_shape
{
    std::string name;

    float specular[4];
    float shininess;

    std::string texture_name;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    std::map<Vertex, unsigned int, VertexComparator> vertex_map;      // only used during load

    indexed_shape(const std::string& name_, const std::string& texture_name_, const float specular_[4], float shininess_) :
        name(name_),
        shininess(shininess_),
        texture_name(texture_name_)
    {
        for(int i = 0; i < 4; i++) specular[i] = specular_[i];
    }
};

typedef std::map<std::string, indexed_shape*> indexed_shape_dict;

bool ReadTriSrc(FILE *fp, std::string _dirname, std::vector<Drawable::sptr>& objects)
{
    indexed_shape_dict shapes;
    char texture_name[512];
    char tag_name[512];
    float specular_color[4];
    float shininess;

    size_t total_tris = 0;

    while(fscanf(fp,"\"%[^\"]\"", texture_name) == 1) {
        if(strcmp(texture_name, "*") == 0)
            texture_name[0] = '\0';

	if(fscanf(fp,"%s ", tag_name) != 1) {
	    fprintf(stderr, "couldn't read tag name\n");
	    return false;
	}

	if(fscanf(fp,"%g %g %g %g %g ", &specular_color[0], &specular_color[1],
	    &specular_color[2], &specular_color[3], &shininess) != 5) {
	    fprintf(stderr, "couldn't read specular properties\n");
	    return false;
	}
	if(shininess > 0 && shininess < 1) {
	    // shininess is not exponent - what is it?
	    shininess *= 10;
	}

        Vertex verts[3];
        for(int i = 0; i < 3; i++) {
	    Vertex& v = verts[i];

	    if(fscanf(fp,"%g %g %g %g %g %g %g %g %g %g %g %g ",
	        &v.v[0], &v.v[1], &v.v[2],
	        &v.n[0], &v.n[1], &v.n[2],
	        &v.c[0], &v.c[1], &v.c[2], &v.c[3],
	        &v.t[0], &v.t[1]) != 12) {

		fprintf(stderr, "couldn't read Vertex\n");
		return false;
	    }
        }

	char shape_name_cstr[512];
	sprintf(shape_name_cstr, "%s.%s%.2f%.2f%.2f%.2f",
            texture_name, tag_name,
            specular_color[0], specular_color[1], specular_color[2],
            shininess);

	std::string shape_name(shape_name_cstr);

	auto itr = shapes.find(shape_name);
	indexed_shape *sh;

	if (itr == shapes.end()) {

            std::string absoluteTextureName;
            if (texture_name == NULL || strlen(texture_name) == 0)
                absoluteTextureName = "";
            else
                absoluteTextureName = _dirname + "/" + std::string(texture_name);

	    sh = new indexed_shape(tag_name,
                absoluteTextureName,
	        specular_color,
		shininess);
	    shapes[shape_name] = sh;

	} else {

	    sh = itr->second;
	}

        for(int i = 0; i < 3; i++) {
	    Vertex& v = verts[i];
	    auto vx = sh->vertex_map.find(v);

	    if(true || vx == sh->vertex_map.end()) {
		sh->vertices.push_back(v);

		int index = sh->vertices.size() - 1;
		sh->vertex_map[v] = index;
		sh->indices.push_back(index);
	    } else {
		sh->indices.push_back(vx->second);
	    }
	}

	total_tris++;
    }

    for(auto itr = shapes.begin(); itr != shapes.end(); itr++) {
        indexed_shape *sh = itr->second;

        static float default_ambient[4] = {.1, .1, .1, 1};
        static float default_diffuse[4] = {1, 1, 1, 1};

        if(sh->texture_name.empty()) {

            PhongShader::Material::sptr mtl(new PhongShader::Material(default_diffuse, default_ambient, sh->specular, sh->shininess));

            // XXX transparency

            objects.push_back(MakeShape(mtl, &sh->vertices[0], sh->vertices.size(), &sh->indices[0], sh->indices.size()));

        } else {

            GLuint texture = LoadTexture(sh->texture_name);
            
            PhongShader::Material::sptr mtl(new PhongShader::Material(default_diffuse, default_ambient, sh->specular, sh->shininess));

            // XXX transparency

            objects.push_back(MakeShape(mtl, &sh->vertices[0], sh->vertices.size(), &sh->indices[0], sh->indices.size()));

        }
    }

    return true;
}

bool Load(const std::string& filename, std::vector<Drawable::sptr>& objects)
{
    FILE *fp = fopen(filename.c_str(), "r");

    if(fp == NULL) {
        fprintf(stderr, "couldn't open \"%s\" for reading\n", filename.c_str());
        return false;
    }

    char filename_copy[filename.size() + 1];
    strncpy(filename_copy, filename.c_str(), filename.size() + 1); // Uhhhhgly.
    std::string _dirname = std::string(dirname(filename_copy));

    bool success = ReadTriSrc(fp, _dirname, objects);

    fclose(fp);

    return success;
}

};

