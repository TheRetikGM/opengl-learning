#pragma once
#include <string>
#include "Mesh.h"
#include <vector>
#include "Shader.h"
#include <assimp/scene.h>

namespace glml
{
	class Model {
	public:
		Model(const std::string path);

		void Draw(Shader& shader) const;
	private:
		// model data
		std::vector<Mesh> meshes;
		std::string directory;
		std::vector<Texture> textures_loaded;

		void loadModel(const std::string path);
		void processNode(aiNode* node, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType glmlType);
	};
	static unsigned int load_texture(const char* path);
	static unsigned int load_texture(const std::string path);
}

