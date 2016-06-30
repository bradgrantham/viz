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

#include <assimp/vector3.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <assimp/config.h>

#include "assimp_loader.h"
#include "phongshader.h"

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

using namespace std;

namespace AssimpLoader
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
    Vertex(const vec3f& v_, const vec3f& n_, const vec4f& c_, const vec2f& t_) :
        v{v_[0], v_[1], v_[2]},
        n{n_[0], n_[1], n_[2]},
        c{c_[0], c_[1], c_[2], c_[3]},
        t{t_[0], t_[1]}
    {}
};

Vertex ConvertVertex(const aiMesh* mesh, int i)
{
    vec3f position(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

    vec3f normal = (mesh->mNormals == NULL) ? vec3f(0, 0, 1) : vec3f(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

    vec2f texcoord = (mesh->mTextureCoords[0] == NULL) ? vec2f(0, 0) : vec2f(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);

    vec4f color = (mesh->mColors[0] == NULL) ? vec4f(1, 1, 1, 1) : vec4f(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b, mesh->mColors[0][i].a);

    return Vertex(position, normal, color, texcoord);
}

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
    drawlist->indexed = true;
    drawlist->indexType = GL_UNSIGNED_INT;
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

tuple<bool, NodePtr> ConvertFacesSmooth(const aiMesh* mesh)
{
    vector<Vertex> vertices;
    vector<unsigned int> indices;

    for(unsigned int j = 0; j < mesh->mNumVertices; j++) {
        vertices.push_back(ConvertVertex(mesh, j));
    }

    for(unsigned int j = 0; j < mesh->mNumFaces; j++) {
        const aiFace& face = mesh->mFaces[j];

        if(face.mNumIndices < 3)
            continue;

        int i0, i1, i2;
        i0 = face.mIndices[0];
        i1 = 0;
        i2 = face.mIndices[1];

        for(unsigned int i = 2; i < face.mNumIndices; i++) {
            i1 = i2;
            i2 = face.mIndices[i];
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);
        }
    }

    static vec4f default_ambient(.1, .1, .1, 1);
    static vec4f default_diffuse(1, 1, 1, 1);

    PhongShader::MaterialPtr mtl(new PhongShader::Material(default_diffuse, default_ambient, vec4f(1, 1, 1, 1), 100));

    return make_tuple(true, MakeShape(mtl, &vertices[0], vertices.size(), &indices[0], indices.size(), false));
}

tuple<bool, NodePtr> ConvertMesh(const aiMesh* mesh)
{
    if(mesh->mNormals == NULL) {
        printf("flat\n");
        return make_tuple(false, NodePtr()); // ConvertFacesFlat(mesh);
    } else {
        return ConvertFacesSmooth(mesh);
    }
}

tuple<bool, GroupPtr> EmitMeshes(const aiScene* scene, const aiNode* node)
{
    vector<NodePtr> children;
    aiMatrix4x4 m = node->mTransformation;
    float mtxf[16];

    aiTransposeMatrix4(&m);
    for(int j = 0; j < 4; j++)
        for(int i = 0; i < 4; i++)
            mtxf[j * 4 + i] = m[j][i];

    // emit all meshes for this transform
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        bool success;
        NodePtr node;
        tie(success, node) = ConvertMesh(mesh);
        if(!success)
            return make_tuple(false, GroupPtr());
        if(node) {
            children.push_back(node);
        }
    }

    // emit all child transformed meshes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        const aiNode* aichild = node->mChildren[i];
        bool success;
        GroupPtr child;
        tie(success, child) = EmitMeshes(scene, aichild);
        if(!success)
            return make_tuple(false, GroupPtr());
        if(child) {
            children.push_back(child);
        }
    }

    if(children.size() > 0)
        return make_tuple(true, GroupPtr(new Group(mtxf, children)));
    else
        return make_tuple(true, GroupPtr());
}

tuple<bool, NodePtr> Load(const string& filename)
{
    const aiScene* scene;
    aiPropertyStore* props = aiCreatePropertyStore();

    int index = filename.find_last_of(".");
    string extension = filename.substr(index + 1);

    if(extension == "stl") {
        // STL files may require this:
        aiSetImportPropertyInteger(props, AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
        scene = aiImportFileExWithProperties(filename.c_str(), aiProcess_RemoveComponent | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_FindDegenerates, NULL, props);
    } else {
        scene = aiImportFileExWithProperties(filename.c_str(), aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_FindDegenerates, NULL, props);
    }

    aiReleasePropertyStore(props);
    printf("aiImportFile completed successfully...\n");
    if(scene == NULL) {
        fprintf(stderr, "couldn't open \"%s\" for reading\n", filename.c_str());
        exit(EXIT_FAILURE);
    }

    bool success;
    GroupPtr child;
    tie(success, child) = EmitMeshes(scene, scene->mRootNode);
    return make_tuple(success, child);
}

};

