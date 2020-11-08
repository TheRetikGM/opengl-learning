#include <glad/glad.h>
#include "Mesh.h"

using namespace glml;

Mesh::Mesh(const std::vector<Vertex>& vertices,
		   const std::vector<unsigned int>& indices,
		   const std::vector<Texture>& textures) : vertices(vertices), indices(indices), textures(textures)
{
	setupMesh();
}

void Mesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, TexCoord));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Tangent));
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, Bitangent));
	for (int i = 0; i < 5; i++)
		glEnableVertexAttribArray(i);

	glBindVertexArray(0);
}

void Mesh::Draw(const Shader& shader) const
{
	unsigned int diffNr = 0;
	unsigned int specNr = 0;
	unsigned int normNr = 0;
	unsigned int heigNr = 0;

	for (int i = 0; i < textures.size(); i++)
	{
		std::string name = TextureTypeStrings[static_cast<size_t>(textures[i].type)];

		if (textures[i].type == TextureType::diffuse)
			name += std::to_string(diffNr++);
		else if (textures[i].type == TextureType::specular)
			name += std::to_string(specNr++);
		else if (textures[i].type == TextureType::normal)
			name += std::to_string(normNr++);
		else if (textures[i].type == TextureType::height)
			name += std::to_string(heigNr++);

		glActiveTexture(GL_TEXTURE0 + i);
		shader.setInt("material." + name, i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}