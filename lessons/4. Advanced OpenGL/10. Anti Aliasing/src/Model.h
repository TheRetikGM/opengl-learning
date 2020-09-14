#pragma once
#include <string>
#include "Mesh.h"
#include <vector>
#include "Shader.h"
#include <assimp/scene.h>

/*
Vertex attributes are expected as follows:
		0. Vertex position
		1. Vertex normal
		2. Vertex texture coordinate
		3. Vertex tangent
		4. Vertex bitangent
In the Fragment Shader there must be atleast
		struct Material
		{
			sampler2D texture_diffuse0;
		}
		uniform Material material;
*/

namespace glml
{
	class Model {
	public:
		std::vector<Mesh> meshes;
		std::vector<Texture> textures_loaded;

		Model(const std::string path);

		void Draw(Shader& shader) const;
	private:
		// model data		
		std::string directory;

		void loadModel(const std::string path);
		void processNode(aiNode* node, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType glmlType);
	};
	static unsigned int load_texture(const char* path);
	static unsigned int load_texture(const std::string path);
}

