#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
	unsigned int Program;

	Shader(const char* vertexShaderSourcePath, const char* fragmentShaderSourcePath);
	Shader(std::string vertexShaderSourcePath, std::string fragmentShaderSourcePath);

	void Use();

	void setBool(const std::string name, bool value) const;
	void setFloat(const std::string name, float value) const;
	void setInt(const std::string name, int value) const;
	void setMat4(const std::string name, glm::mat4 value) const;
	void setVec3(const std::string name, glm::vec3 value) const;
	void setVec3(const std::string name, float v0, float v1, float v2) const;
private:
	std::string readFile(const char* path);	
};

