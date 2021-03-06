#pragma once

/**
* Skeletal Animation Learning Resources
* https://realitymultiplied.wordpress.com/2016/05/02/assimp-skeletal-animation-tutorial-1-vertex-weights-and-indices/
* http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html
* https://github.com/vovan4ik123/assimp-Cpp-OpenGL-skeletal-animation
*/

// glm
#include <glm/glm.hpp>

// assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

// opengl
#include "opengl.hpp"

// std
#include <iostream>
#include <map>

// project
#include "cgra/cgra_mesh.hpp"
#include "boids_bone.hpp"

using namespace glm;
using namespace std;
using namespace cgra;

#define NUM_BONES_PER_VERTEX 4

namespace blukzen
{
    struct bone_info
    {
        aiMatrix4x4 offset;
        aiMatrix4x4 transform;
    };

    struct vertex_bone
    {
        unsigned int ids[NUM_BONES_PER_VERTEX] {0, 0, 0, 0};
        float weights[NUM_BONES_PER_VERTEX] {0, 0, 0, 0};

        /**
         * Append given data to ids & weights
         * @param id
         * @param weight
         */
        void add_bone_data(unsigned int id, float weight)
        {
            for (int i = 0; i < sizeof(ids) / sizeof(unsigned int); i++)
            {
                if (weights[i] == 0.0)
                {
                    ids[i] = id;
                    weights[i] = weight;
                    return;
                }
            }
        }
    };
    
    struct vertex
    {
        vec3 pos {0};
        vec3 norm {0};
        vec2 uv {0};
        vec4 ids {0};
        vec4 weights {0};
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

    
    class boids_model
    {
        vec3 color_ {0.7, 0, 0};
        GLuint shader_ = 0;
        vector<gl_mesh> meshes_;
        vector<aiNodeAnim*> animations_;
        vector<bone_info> bones_info_;
        vector<bone> bones_;

        Assimp::Importer import_;
        const aiScene* scene_;
        aiMatrix4x4 global_inverse_transform;
        map<string, unsigned int> bone_map_; // Maps vertex bone data to bone info

        float animation_duration_ = 0;
        float animation_ticks_ = 0;
        unsigned int number_of_bones_ = 0;
        
    public:
        boids_model();
        ~boids_model();
        void draw(const mat4& view, const mat4 proj);

        // Model processing
        void load_model(const string& filename);
        void process_node(aiNode* node, const aiScene* scene);
        void process_anim(const aiScene* scene);
        gl_mesh process_mesh(aiMesh* mesh, const aiScene* scene);

        // Animation
        void bone_transform(float time_seconds, vector<mat4>& transforms);
        void bone_update_transform(float animation_time, aiNode* node, aiMatrix4x4 identity);
        aiNodeAnim* find_node_anim(string node_name);
        aiQuaternion calc_interpolated_rotation(float animation_time, const aiNodeAnim* node_anim);
        uint find_position(float animation_time, const aiNodeAnim* anim);
        uint find_rotation(float animation_time, const aiNodeAnim* anim);
        static mat4 ai_to_mat4(aiMatrix4x4 in);
    };



}