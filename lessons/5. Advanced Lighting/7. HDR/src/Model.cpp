#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include <stb_image.h>
#include "Model.h"
#include "DebugColors.h"

using namespace glml;

Model::Model(const std::string path, bool flip_textures, bool sRGB_diffuse) :
	sRGB_diffuse(sRGB_diffuse), flip_textures(flip_textures)
{
	loadModel(path);
}
void Model::Draw(const Shader& shader) const
{
	for (int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader);
}
void Model::loadModel(const std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << DC_ERROR "[ASSIMP] Model '" << path << "' not loaded.\n" << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}
void Model::processNode(aiNode* node, const aiScene* scene)
{
	unsigned int i;
	for (i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	for (i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);
}
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	unsigned int i;

	for (i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 v;
		v.x = mesh->mVertices[i].x;
		v.y = mesh->mVertices[i].y;
		v.z = mesh->mVertices[i].z;
		vertex.Position = v;

		if (mesh->mNormals != NULL)
		{
			v.x = mesh->mNormals[i].x;
			v.y = mesh->mNormals[i].y;
			v.z = mesh->mNormals[i].z;
			vertex.Normal = v;
		}

		if (mesh->mTangents != NULL)
		{
			v.x = mesh->mTangents[i].x;
			v.y = mesh->mTangents[i].y;
			v.z = mesh->mTangents[i].z;
			vertex.Tangent = v;
		}
		if (mesh->mBitangents != NULL)
		{
			v.x = mesh->mBitangents[i].x;
			v.y = mesh->mBitangents[i].y;
			v.z = mesh->mBitangents[i].z;
			vertex.Bitangent = v;
		}

		if (mesh->mTextureCoords[0]) {	// does the mesh contains texture coordinates?
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoord = vec;
		}
		else {
			vertex.TexCoord = glm::vec2(0.0f);
		}
		vertices.push_back(vertex);
	}
	for (i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, TextureType::diffuse);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, TextureType::specular);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		std::vector<Texture> normalMaps = loadMaterialTextures(mat, aiTextureType_NORMALS, TextureType::normal);
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<Texture> heightMaps = loadMaterialTextures(mat, aiTextureType_HEIGHT, TextureType::height);
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
	}
	return Mesh(vertices, indices, textures);
}
std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType glmlType)
{
	std::vector<Texture> textures;
	bool sRGB = false;
	if (sRGB_diffuse && glmlType == TextureType::diffuse)
		sRGB = true;

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool loaded = false;

		for (int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				loaded = true;
				break;
			}
		}
		if (!loaded)
		{
			Texture texture;			
			texture.id = load_texture(directory + '/' + str.C_Str(), flip_textures, sRGB);
			texture.type = glmlType;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
	return textures;
}

static unsigned int glml::load_texture(std::string path, bool flip, bool sRGB, GLint wrapS, GLint wrapT)
{
	return glml::load_texture(path.c_str(), flip, sRGB, wrapS, wrapT);
}
static unsigned int glml::load_texture(const char* path, bool flip, bool sRGB, GLint wrapS, GLint wrapT)
{
	unsigned int texture = 0;
	int width, height, nrChannels, format, intFormat;
	stbi_set_flip_vertically_on_load(flip);
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);		// load image to the array of bytes (char = 1 byte)	

	switch (nrChannels)
	{
	case 4: format = GL_RGBA;
		if (sRGB) intFormat = GL_SRGB_ALPHA;
		else intFormat = GL_RGBA;
		break;
	case 3: format = GL_RGB;
		if (sRGB) intFormat = GL_SRGB;
		else intFormat = GL_RGB;
		break;
	case 2: format = intFormat = GL_RED; break;
	default: format = intFormat = GL_RGB; break;
	}

	if (data) // data != NULL
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);	// generates texture
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cerr << DC_WARNING " Could not load texture '" << path << "'\n";
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}