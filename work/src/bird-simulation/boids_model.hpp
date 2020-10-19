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

namespace blukzen
{
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

    struct vertex
    {
        glm::vec3 pos {0};
        glm::vec3 norm {0};
        glm::vec2 uv {0};
    };

    struct model_builder
    {
        GLenum mode = GL_TRIANGLES;
        vector<vertex> vertices;
        vector<unsigned int> indices;

        model_builder() {}
        explicit model_builder(const GLenum mode) : mode(mode) {}

        template <size_t N, size_t M>
        explicit model_builder(const vertex(&vert_data)[N], const vertex(&idx_data)[M], const GLenum mode = GL_TRIANGLES)
        : mode(mode), vertices(vert_data, vert_data+N), indices(idx_data, idx_data+M) { }

        GLuint push_vertex(vertex v) {
            auto size = vertices.size();
            assert(size == decltype(size)(GLuint(size)));
            vertices.push_back(v);
            return GLuint(size);
        }

        void push_index(GLuint i) {
            indices.push_back(i);
        }

        void push_indices(std::initializer_list<GLuint> inds) {
            indices.insert(indices.end(), inds);
        }

        gl_mesh build() const;

        void print() const {
            std::cout << "pos" << std::endl;
            for (auto v : vertices) {
                std::cout << v.pos.x << ", " << v.pos.y << ", " << v.pos.z << ", ";
                std::cout << v.norm.x << ", " << v.norm.y << ", " << v.norm.z << ", ";
                std::cout << v.uv.x << ", " << v.uv.y << ", " << std::endl;
            }
            std::cout << "idx" << std::endl;
            for (int i : indices) {
                std::cout << i << ", ";
            }
            std::cout << std::endl;
        }
    };
}