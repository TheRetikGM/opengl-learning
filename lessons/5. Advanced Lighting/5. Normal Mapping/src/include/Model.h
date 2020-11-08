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
In the Fragment Shader there must be at least
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

		/* sRGB_diffuse: when true, save diffuse textures as sRGB textures*/
		Model(const std::string path, bool flip_textures = false, bool sRGB_diffuse = false);		

		void Draw(const Shader& shader) const;
	private:
		// model data		
		std::string directory;
		const bool sRGB_diffuse;
		const bool flip_textures;

		void loadModel(const std::string path);
		void processNode(aiNode* node, const aiScene* scene);
		Mesh processMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType glmlType);
	};
	static unsigned int load_texture(const char* path, bool flip = true, bool sRGB = true, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT);
	static unsigned int load_texture(std::string path, bool flip = true, bool sRGB = true, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT);
}

