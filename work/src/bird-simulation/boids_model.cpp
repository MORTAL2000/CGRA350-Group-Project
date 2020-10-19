// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// std
#include <iostream>
#include <map>

// opengl
#include "opengl.hpp"

// project
#include "boids_model.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_mesh.hpp"

using namespace glm;
using namespace cgra;
using namespace std;
using namespace blukzen;

boids_model::boids_model()
{
    // Load model shaders
    shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//boid_vert.glsl"));
    sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//boid_frag.glsl"));
    const auto shader = sb.build();
    shader_ = shader;
}

/**
 * Draws this model.
 * TODO: Add parameters for translation & rotation
 * @param view View matrix.
 * @param proj Projection matrix.
 */
void boids_model::draw(const mat4& view, const mat4 proj)
{
    auto modelview = view;

    glUseProgram(shader_); // load shader and variables
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uProjectionMatrix"), 1, false, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uModelViewMatrix"), 1, false, value_ptr(modelview));
    glUniform3fv(glGetUniformLocation(shader_, "uColor"), 1, value_ptr(color_));

    if (!meshes_.empty()) {
        for each (gl_mesh mesh in meshes_)
        {
            mesh.draw();
        }
    }
}

/**
 * Loads the given model using the asset importer library.
 * @param filename Path to the asset to load.
 */
void boids_model::load_model(const string& filename)
{
    // Read file and triangulate, flip UVs because opengl reveresed around y-axis.
    Assimp::Importer importer;
    const auto scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

    // Make sure the file was imported/didnt error.
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "Assimp failed to load model: " << filename << endl << importer.GetErrorString() << endl;
        return;
    }

    process_node(scene->mRootNode, scene);

    cout << "Loaded " << meshes_.size() << " meshes" << endl;
}

/**
 * Recursively processes nodes in an aiScene
 * @param node Current node to process.
 * @param scene Parent scene.
 */
void boids_model::process_node(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(process_mesh(mesh, scene));
    }

    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        process_node(node->mChildren[i], scene);
    }
}

/**
 * Given an aiMesh builds a gl_mesh using cgra::mesh_builder
 * @param mesh Mesh to build.
 * @param scene Parent scene of the mesh.
 */
gl_mesh boids_model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
    vector<vertex> vertices;
    vector<unsigned int> indices;
    vector<vertex_bone> bones;

    // This is used to get the vertices that are pointed to, by the bones.
    auto base_vertex_index = 0;
    const auto vertex_size = 5;

    vertices.resize(mesh->mNumVertices);
    bones.resize(mesh->mNumVertices);
    

    // Process mesh data
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // Positions
        vertices[i].pos.x = mesh->mVertices[i].x;
        vertices[i].pos.y = mesh->mVertices[i].y;
        vertices[i].pos.z = mesh->mVertices[i].z;

        // Normals
        if (mesh->HasNormals()) {
            vertices[i].norm.x = mesh->mNormals[i].x;
            vertices[i].norm.y = mesh->mNormals[i].y;
            vertices[i].norm.z = mesh->mNormals[i].z;
        }

        // UVs
        if (mesh->mTextureCoords[0]) {
            vertices[i].uv.x = mesh->mTextureCoords[0][i].x;
            vertices[i].uv.y = mesh->mTextureCoords[0][i].y;
        }
        else {
            vertices[i].uv = vec2(0.0f, 0.0f);
        }
    }

    // Bone data
    if (mesh->HasBones())
    {
        for (int j = 0; j < mesh->mNumBones; j++)
        {
            auto bone_index = 0;
            auto bone_name = string(mesh->mBones[j]->mName.data);

            if (bone_map_.find(bone_name) == bone_map_.end())
            {
                bone_index = number_of_bones;
                number_of_bones++;

                // TODO: BONE INFO MAPPING
            }

            for (int k = 0; k < mesh->mBones[j]->mNumWeights; k++)
            {
                auto vert_id = mesh->mBones[j]->mWeights[k].mVertexId;
                float weight = mesh->mBones[j]->mWeights[k].mWeight;

                bones[vert_id].add_bone_data(vert_id, weight);
                    
                //unsigned int local_vert_id = mesh->mBones[j]->mWeights[k].mVertexId;
            }
        }

        // Add bone info to vertex data
        for (int m = 0; m < mesh->mNumVertices; m++)
        {
            vertices[m].ids = vec4(bones[m].ids[0], bones[m].ids[1], bones[m].ids[2], bones[m].ids[3]);
            vertices[m].weights = vec4(bones[m].weights[0], bones[m].weights[1], bones[m].weights[2], bones[m].weights[3]);
        }
        
    }

    // Indices
    // Walk through each of mesh's faces and retrieve vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        auto face = mesh->mFaces[i];

        // Retrieve all indices of the face.
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Build mesh
    model_builder mb;
    
    for (unsigned int i = 0; i < vertices.size(); i++) {
        mb.push_index(indices[i]);
        mb.push_vertex(vertices[i]);
    }

    return mb.build();
}

gl_mesh model_builder::build() const {

    gl_mesh m;
    glGenVertexArrays(1, &m.vao); // VAO stores information about how the buffers are set up
    glGenBuffers(1, &m.vbo); // VBO stores the vertex data
    glGenBuffers(1, &m.ibo); // IBO stores the indices that make up primitives

    // VAO
    glBindVertexArray(m.vao);
    // VBO (single buffer, interleaved)
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    // upload ALL the vertex data in one buffer
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_STATIC_DRAW);

    // this buffer will use location=0 when we use our VAO
    glEnableVertexAttribArray(0);
    // tell opengl how to treat data in location=0 - the data is treated in lots of 3 (3 floats = vec3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(offsetof(vertex, pos)));

    // do the same thing for Normals but bind it to location=1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(offsetof(vertex, norm)));

    // do the same thing for UVs but bind it to location=2 - the data is treated in lots of 2 (2 floats = vec2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)(offsetof(vertex, uv)));

    // Bone ids
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_INT, GL_FALSE, sizeof(vertex), (void *)(offsetof(vertex, ids)));

    // Bone weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_TRUE, sizeof(vertex), (void *)(offsetof(vertex, weights)));

    // IBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo);
    // upload the indices for drawing primitives
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);


    // set the index count and draw modes
    m.index_count = indices.size();
    m.mode = mode;

    // clean up by binding VAO 0 (good practice)
    glBindVertexArray(0);

    return m;
}
