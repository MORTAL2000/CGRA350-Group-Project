#pragma once

// glm
#include <glm/glm.hpp>

// assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

// opengl
#include "opengl.hpp"

// std
#include <iostream>

// project
#include "cgra/cgra_mesh.hpp"

using namespace glm;
using namespace std;
using namespace cgra;

class boids_model
{
    vec3 color_ {0.7, 0, 0};
    GLuint shader_ = 0;
    vector<gl_mesh> meshes_;

public:
    boids_model();
    void draw(const mat4& view, const mat4 proj);
    void load_model(const string& filename);
    void process_node(aiNode* node, const aiScene* scene);
    static gl_mesh process_mesh(aiMesh* mesh, const aiScene* scene);
};