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
#include <iostream>
#include <map>
#include <libgen.h>
#include <FreeImagePlus.h>
#include "trisrc_loader.h"
#include "phongshader.h"

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

using namespace std;

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

GLuint LoadTexture(const string& filename)
{
    GLuint texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    CheckOpenGL(__FILE__, __LINE__);

    fipImage image;
    bool success;

    if (!(success = image.load(filename.c_str()))) {

        cerr << "LoadTexture: Failed to load image from " <<
            filename << endl;

    } else if (!(success = image.convertTo32Bits())) {

        cerr << "LoadTexture: Couldn't convert image to 24 bits " <<
            filename << endl;

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
            cerr << "Unhandled FIP image type: " << image.getImageType() << endl;
            success = false;

        } else {

            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.getWidth(), image.getHeight(), 0, sourceFormat, sourceType, image.accessPixels());
            glGenerateMipmap(GL_TEXTURE_2D);
            CheckOpenGL(__FILE__, __LINE__);
        }

    }

    if (!success) {
        cerr << "GL Renderer: loading checkerboard instead." << endl;
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
    Vertex() {}
    Vertex(float v_[3], float n_[3], float c_[4], float t_[2]) :
        v{v_[0], v_[1], v_[2]},
        n{n_[0], n_[1], n_[2]},
        c{c_[0], c_[1], c_[2], c_[3]},
        t{t_[0], t_[1]}
    {}
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


NodePtr MakeShape(PhongShader::MaterialPtr mtl, Vertex *vertices, size_t vertexCount, unsigned int *indices, int indexCount, bool textured)
{
    PhongShaderPtr shader = PhongShader::GetForCurrentContext();

    PhongShader::ProgramVariant& vt = textured ? shader->textured : shader->nontextured;
    glUseProgram(vt.program);

    DrawListPtr drawlist(new DrawList);
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
    size_t texcoordOffset = colorOffset + colorSize;

    glVertexAttribPointer(vt.positionAttrib, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(vt.positionAttrib);

    glVertexAttribPointer(vt.normalAttrib, 3, GL_FLOAT, GL_FALSE, stride, (void*)normalOffset);
    glEnableVertexAttribArray(vt.normalAttrib);

    glVertexAttribPointer(vt.colorAttrib, 4, GL_FLOAT, GL_FALSE, stride, (void*)colorOffset);
    glEnableVertexAttribArray(vt.colorAttrib);

    if(textured) {
        glVertexAttribPointer(vt.texcoordAttrib, 2, GL_FLOAT, GL_FALSE, stride, (void*)texcoordOffset);
        glEnableVertexAttribArray(vt.texcoordAttrib);
        CheckOpenGL(__FILE__, __LINE__);
    }

    glBindVertexArray(GL_NONE);

    box bounds;
    for(int i = 0; i < vertexCount; i++)
        bounds.extend(vertices[i].v[0], vertices[i].v[1], vertices[i].v[2]);

    DrawablePtr drawable(new PhongShadedGeometry(drawlist, mtl, bounds));
    return ShapePtr(new Shape(drawable));
}

struct indexed_shape
{
    string name;

    vec4f specular;
    float shininess;

    string texture_name;

    vector<Vertex> vertices;
    vector<unsigned int> indices;

    void add_triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
    {
        for(auto v : {v0, v1, v2}) {
	    auto vx = vertex_map.find(v);

	    if(true /* vx == vertex_map.end() */) {
		vertices.push_back(v);

		int index = vertices.size() - 1;
		vertex_map[v] = index;
		indices.push_back(index);

	    } else {
		indices.push_back(vx->second);
	    }
	}
    }

    map<Vertex, unsigned int, VertexComparator> vertex_map;      // only used during load

    indexed_shape(const string& name_, const string& texture_name_, const vec4f& specular_, float shininess_) :
        name(name_),
        specular(specular_),
        shininess(shininess_),
        texture_name(texture_name_)
    { }

};

typedef map<string, indexed_shape*> indexed_shape_dict;


/*

VERTEX
    VERTEX(float v[3], float n[3], float c[4], float t[2])
MATERIAL
    MATERIAL(const char *texture, float specular[4], float shininess);
TRIANGLE_SETS
    get(const char *name, const MATERIAL& material)
        add_triangle(const VERTEX& v0, const VERTEX& v1, const VERTEX& v2)

*/

template <class TRIANGLE_SETS, class MATERIAL, class VERTEX>
bool ParseTriSrc(FILE *fp, TRIANGLE_SETS& sets)
{
    char texture_name[512];
    char tag_name[512];
    float specular_color[4];
    float shininess;

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

        VERTEX verts[3];
        for(int i = 0; i < 3; i++) {
            float v[3];
            float n[3];
            float c[4];
            float t[2];

	    if(fscanf(fp,"%g %g %g %g %g %g %g %g %g %g %g %g ",
	        &v[0], &v[1], &v[2],
	        &n[0], &n[1], &n[2],
	        &c[0], &c[1], &c[2], &c[3],
	        &t[0], &t[1]) != 12) {

		fprintf(stderr, "couldn't read Vertex\n");
		return false;
	    }
            verts[i] = VERTEX(v, n, c, t);
        }

        MATERIAL mtl(texture_name, specular_color, shininess);

        sets.get_triangle_set(tag_name, mtl).add_triangle(verts[0], verts[1], verts[2]);
    }
    return true;
}

// XXX Temporary scaffolding to support parsing TriSets with old goop
struct material
{
    string diffuse_texture_name;
    float specular[4];
    float shininess;
    material(const string diffuse_texture, float specular_[4], float shininess_) :
        diffuse_texture_name(diffuse_texture),
        specular{specular_[0], specular_[1], specular_[2], specular_[3]},
        shininess(shininess_)
    { }
};

struct triangle_sets
{
    string _dirname;
    triangle_sets(const string dirname_) :
        _dirname(dirname_)
    {}

    indexed_shape_dict shapes;
    indexed_shape& get_triangle_set(const char *name, const material& mtl)
    {
        char shape_name_cstr[512];
        sprintf(shape_name_cstr, "%s.%s%.2f%.2f%.2f%.2f",
            mtl.diffuse_texture_name.c_str(), name,
            mtl.specular[0], mtl.specular[1], mtl.specular[2],
            mtl.shininess);

        string shape_name(shape_name_cstr);

        auto itr = shapes.find(shape_name);
        indexed_shape *sh;

        if (itr == shapes.end()) {

            string absoluteTextureName;
            if (mtl.diffuse_texture_name.empty())
                absoluteTextureName = "";
            else
                absoluteTextureName = _dirname + "/" + string(mtl.diffuse_texture_name);

            sh = new indexed_shape(name, absoluteTextureName, mtl.specular,
                mtl.shininess);
            shapes[shape_name] = sh;

        } else {

            sh = itr->second;

        }
        return *sh;
    }
};

// XXX let the thing this is calling do the de-indexing?
bool ReadTriSrc(FILE *fp, string _dirname, vector<NodePtr>& nodes)
{
    triangle_sets sets(_dirname);

    bool success = ParseTriSrc<triangle_sets, material, Vertex>(fp, sets);

    if(!success)
        return success;

    for(auto named_shape : sets.shapes) {
        indexed_shape *sh = named_shape.second;

        static vec4f default_ambient(.1, .1, .1, 1);
        static vec4f default_diffuse(1, 1, 1, 1);

        if(sh->texture_name.empty()) {

            PhongShader::MaterialPtr mtl(new PhongShader::Material(default_diffuse, default_ambient, sh->specular, sh->shininess));

            // XXX transparency

            nodes.push_back(MakeShape(mtl, &sh->vertices[0], sh->vertices.size(), &sh->indices[0], sh->indices.size(), false));

        } else {

            GLuint texture = LoadTexture(sh->texture_name);
            
            PhongShader::MaterialPtr mtl(new PhongShader::Material(default_diffuse, texture, default_ambient, sh->specular, sh->shininess));

            // XXX transparency

            nodes.push_back(MakeShape(mtl, &sh->vertices[0], sh->vertices.size(), &sh->indices[0], sh->indices.size(), true));
        }
    }

    return true;
}

tuple<bool, NodePtr> Load(const string& filename)
{
    FILE *fp = fopen(filename.c_str(), "r");

    if(fp == NULL) {
        fprintf(stderr, "couldn't open \"%s\" for reading\n", filename.c_str());
        return false;
    }

    char filename_copy[filename.size() + 1];
    strncpy(filename_copy, filename.c_str(), filename.size() + 1);
    string _dirname = string(dirname(filename_copy));

    vector<NodePtr> nodes;
    bool success = ReadTriSrc(fp, _dirname, nodes);

    fclose(fp);

    if(!success)
        return make_tuple(success, GroupPtr());

    GroupPtr group(new Group(mat4f::identity, nodes));

    return make_tuple(success, group);
}

};

