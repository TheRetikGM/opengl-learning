#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Shader.h"

namespace glml 
{
	static const std::string TextureTypeStrings[] = {
		"texture_diffuse",
		"texture_specular",
		"texture_normal",
		"texture_height"
	};
	enum class TextureType : size_t
	{
		diffuse = 0,
		specular = 1,
		normal = 2,
		height = 3
	};

	struct Vertex {
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
		glm::vec3 Color;
		glm::vec3 Tangent;
		glm::vec3 Bitangent;
	};
	struct Texture {
		unsigned int id;
		TextureType type;
		std::string path;
	};

	class Mesh {
	public:
		unsigned int VAO;

		std::vector<Vertex>			vertices;
		std::vector<unsigned int>	indices;
		std::vector<Texture>		textures;

		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures);

		void Draw(Shader& shader) const;
	private:
		unsigned int VBO, EBO;

		void setupMesh();
	};
};

