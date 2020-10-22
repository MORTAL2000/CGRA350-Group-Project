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
    scene_ = nullptr;
}

boids_model::~boids_model()
{
    // Clears importer pointers
    import_.FreeScene();
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
    vector<mat4> transforms;
    bone_transform(glfwGetTime(), transforms);

    //cout << glfwGetTime() << " - " << transforms[0][0].a << endl;

    glUseProgram(shader_); // load shader and variables
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uProjectionMatrix"), 1, false, value_ptr(proj));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uModelViewMatrix"), 1, false, value_ptr(modelview));
    glUniformMatrix4fv(glGetUniformLocation(shader_, "uBones"), transforms.size(), GL_TRUE, (GLfloat*)&transforms);
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
    scene_ = import_.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

    // Make sure the file was imported/didnt error.
    if (!scene_ || scene_->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene_->mRootNode) {
        cout << "Assimp failed to load model: " << filename << endl << import_.GetErrorString() << endl;
    }

    if (scene_->mAnimations[0]->mTicksPerSecond != 0.0)
    {
        animation_ticks_ = scene_->mAnimations[0]->mTicksPerSecond;
    } else
    {
        animation_ticks_ = 25.0f;
    }

    animation_duration_ = scene_->mAnimations[0]->mDuration;

    global_inverse_transform = scene_->mRootNode->mTransformation;
    global_inverse_transform.Inverse();

    process_node(scene_->mRootNode, scene_);
    process_anim(scene_);

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

void boids_model::process_anim(const aiScene* scene)
{
    if(scene->mNumAnimations == 0)
        return;

    for(int i = 0; i < scene->mAnimations[0]->mNumChannels; i++)
    {
        animations_.push_back(scene->mAnimations[0]->mChannels[i]);
    }

    cout << "Loaded animations" << endl;
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
                bone_index = bones_info_.size();
                bones_info_.push_back(bone_info{mesh->mBones[j]->mOffsetMatrix});
                bone_map_[bone_name] = bone_index;
                
            } else
            {
                bone_index = bone_map_[bone_name];
            }

            for (int k = 0; k < mesh->mBones[j]->mNumWeights; k++)
            {
                auto vert_id = mesh->mBones[j]->mWeights[k].mVertexId;
                float weight = mesh->mBones[j]->mWeights[k].mWeight;

                bones[vert_id].add_bone_data(vert_id, weight);
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

/**
 * Returns the transforms for bone_info at a given time in the current animation.
 * @param time_seconds Time in the animation.
 * @param transforms Out for transforms.
 */
void boids_model::bone_transform(float time_seconds, vector<mat4>& transforms)
{
    aiMatrix4x4 identity;
    
    double tps = 25.0f;
    float tick = time_seconds * tps;
    float animation_time = fmod(tick, animation_duration_);

    aiNode* root = scene_->mRootNode;

    bone_update_transform(animation_time, scene_->mRootNode, identity);
    transforms.resize(bones_info_.size());

    for (int i = 0; i < bones_info_.size(); i++)
    {
        transforms[i] = ai_to_mat4(bones_info_[i].transform);
    }
}

void boids_model::bone_update_transform(float animation_time, aiNode* node, aiMatrix4x4 identity)
{
    string node_name(node->mName.data);
    aiMatrix4x4 node_transform = node->mTransformation;

    const aiAnimation* animation = scene_->mAnimations[0];
    const aiNodeAnim* node_anim = find_node_anim(node_name);

    if (node_anim)
    {
        // Rotation
        aiQuaternion rotation_quat = calc_interpolated_rotation(animation_time, node_anim);
        aiMatrix4x4 rotation_mat = aiMatrix4x4(rotation_quat.GetMatrix());

        node_transform = rotation_mat;
    }

    // TODO: Global inverse matrix
    aiMatrix4x4 global_transform = identity * node_transform;

    // True if node_name exists in map
    if (bone_map_.find(node_name) != bone_map_.end())
    {
        uint bone_index = bone_map_[node_name];
        bones_info_[bone_index].transform = global_inverse_transform * global_transform * bones_info_[bone_index].offset;
    }

    // Recusively update children's transform
    for (int i = 0; i < node->mNumChildren; i++)
    {
        bone_update_transform(animation_time, node->mChildren[i], global_transform);
    }
}

aiNodeAnim* boids_model::find_node_anim(string node_name)
{
    for (aiNodeAnim* anim : animations_)
    {
        if (string(anim->mNodeName.data) == node_name)
            return anim;
    }

    return nullptr;
}

aiQuaternion boids_model::calc_interpolated_rotation(float animation_time, const aiNodeAnim* node_anim)
{
    // If there's only one rotation key just return that.
    if (node_anim->mNumRotationKeys == 1)
    {
        return node_anim->mRotationKeys[0].mValue;
    }

    // Calculate interpolated value given 2 key frames and current time
    uint rotation_index = find_rotation(animation_time, node_anim); // Current key frame
    uint next_rotation_index = rotation_index + 1; // Next key frame

    // Index check
    if (next_rotation_index > node_anim->mNumRotationKeys)
    {
        cout << "NEXT ROTATION INDEX LARGER THAN NUMBER OF ROTATION KEYS" << endl;
        return aiQuaternion();
    }

    // Get the time between the two frames
    float delta_time = node_anim->mRotationKeys[next_rotation_index].mTime - node_anim->mRotationKeys[rotation_index].mTime;
    // Get the interpolation factor/Normalized time between frames
    float factor = (animation_time - node_anim->mRotationKeys[rotation_index].mTime) / delta_time;

    const aiQuaternion& start_rotation = node_anim->mRotationKeys[rotation_index].mValue;
    const aiQuaternion& end_rotation = node_anim->mRotationKeys[next_rotation_index].mValue;

    aiQuaternion final_rotation;
    aiQuaternion::Interpolate(final_rotation, start_rotation, end_rotation, factor);
    return final_rotation.Normalize();
}

/**
* Returns the position key frame for a given time in an animation.
* @param animation_time Time in animation.
* @param anim Animation.
*/
uint boids_model::find_position(float animation_time, const aiNodeAnim* anim)
{
    for (uint i = 0; i < anim->mNumPositionKeys -1; i++)
    {
        if (animation_time < (float) anim->mPositionKeys[i + 1].mTime)
        {
            return i;
        }
    }

    return 0; // Should not reach here.
}

/**
 * Returns the rotation key frame for a given time in an animation.
 * @param animation_time Time in animation.
 * @param anim Animation.
 */
uint boids_model::find_rotation(float animation_time, const aiNodeAnim* anim)
{
    for (uint i = 0; i < anim->mNumRotationKeys -1; i++)
    {
        if (animation_time < (float) anim->mRotationKeys[i + 1].mTime)
        {
            return i;
        }
    }

    return 0; // Should not reach here.
}

/**
 * Given an aiMatrix4x4 returns a glm mat4.
 * @param in The aiMatrix4x4 to convert.
 */
mat4 boids_model::ai_to_mat4(aiMatrix4x4 in)
{
    mat4 temp;
    
    temp[0][0] = in.a1;
    temp[1][0] = in.b1;
    temp[2][0] = in.c1;
    temp[3][0] = in.d1;

    temp[0][1] = in.a2;
    temp[1][1] = in.b2;
    temp[2][1] = in.c2;
    temp[3][1] = in.d2;

    temp[0][2] = in.a3;
    temp[1][2] = in.b3;
    temp[2][2] = in.c3;
    temp[3][2] = in.d3;

    temp[0][3] = in.a4;
    temp[1][3] = in.b4;
    temp[2][3] = in.c4;
    temp[3][3] = in.d4;

    return temp;
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


    // set the index count and draw modesvoid process_anim(aiNo);aconst aiScene(*( scenevector<aiNodeAnim*> amo,s;nimatianimations;_
    m.index_count = indices.size();
    m.mode = mode;

    // clean up by binding VAO 0 (good practice)
    glBindVertexArray(0);

    return m;
}
