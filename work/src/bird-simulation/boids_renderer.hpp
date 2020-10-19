#pragma once

// glm
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// project
#include "cgra/cgra_mesh.hpp"
#include "opengl.hpp"
#include <iostream>

using namespace glm;
using namespace std;
using namespace cgra;

class BoidsRenderer {
private:
	glm::vec3 m_color{ 0.7 };
	GLuint m_shader = 0;

public:
	vector<gl_mesh> meshes;

	BoidsRenderer();
	void draw(const glm::mat4& view, const glm::mat4 proj);

	inline void load_model(const std::string& filename) {
		// Read file and triangulate, flip UVs because opengl reveresed around y-axis.
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

		// Make sure the file was imported/didnt error.
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "Assimp failed to load model: " << filename << endl << importer.GetErrorString() << endl;
			return;
		}

		processNode(scene->mRootNode, scene);

		cout << "Loaded " << meshes.size() << " meshes" << endl;
	}

	inline void processNode(aiNode* node, const aiScene* scene) {
		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}

		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

	inline gl_mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		vector<vec3> positions;
		vector<vec3> normals;
		vector<vec2> uvs;
		vector<unsigned int> indices;

		// Process mesh data
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			glm::vec3 v;

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
			glm::vec2 uv;

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
			aiFace face = mesh->mFaces[i];

			// Retrieve all indices of the face.
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		// Build mesh
		mesh_builder mb;

		for (unsigned int i = 0; i < positions.size(); i++) {
			mb.push_index(indices[i]);
			mb.push_vertex(mesh_vertex{
				positions[i],
				normals[i],
				uvs[i]
			});
		}

		return mb.build();
	}
};