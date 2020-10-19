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

// opengl
#include "opengl.hpp"

// project
#include "boids_model.hpp"
#include "cgra/cgra_shader.hpp"
#include "cgra/cgra_mesh.hpp"

using namespace glm;
using namespace cgra;
using namespace std;

boids_model::boids_model()
{
    // Load model shaders
    shader_builder sb;
    sb.set_shader(GL_VERTEX_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_vert.glsl"));
    sb.set_shader(GL_FRAGMENT_SHADER, CGRA_SRCDIR + std::string("//res//shaders//color_frag.glsl"));
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
    vector<vec3> positions;
    vector<vec3> normals;
    vector<vec2> uvs;
    vector<unsigned int> indices;

    // Process mesh data
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vec3 v;

        // Positions
        v.x = mesh->mVertices[i].x;
        v.y = mesh->mVertices[i].y;
        v.z = mesh->mVertices[i].z;
        positions.push_back(v);

        // Normals
        if (mesh->HasNormals()) {
            v.x = mesh->mNormals[i].x;
            v.y = mesh->mNormals[i].y;
            v.z = mesh->mNormals[i].z;

            normals.push_back(v);
        }

        // UVs
        vec2 uv;

        if (mesh->mTextureCoords[0]) {
            uv.x = mesh->mTextureCoords[0][i].x;
            uv.y = mesh->mTextureCoords[0][i].y;
        }
        else {
            uv = vec2(0.0f, 0.0f);
        }

        uvs.push_back(uv);
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
    mesh_builder mb;

    for (unsigned int i = 0; i < positions.size(); i++) {
        mb.push_index(indices[i]);
        mb.push_vertex(mesh_vertex{positions[i],normals[i], uvs[i]});
    }

    return mb.build();
}
