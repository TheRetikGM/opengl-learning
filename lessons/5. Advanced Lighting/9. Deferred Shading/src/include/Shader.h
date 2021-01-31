#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
	unsigned int Program;

	Shader(const char* vertexShaderSourcePath, const char* fragmentShaderSourcePath, const char* geometryShaderSource = NULL, bool directSource = false);
	Shader(std::string vertexShaderSourcePath, std::string fragmentShaderSourcePath, std::string geometryShaderSource = NULL, bool directSource = false);
	~Shader();


	void Use() const;

	void setBool(const std::string name, bool value) const;
	void setFloat(const std::string name, float value) const;
	void setInt(const std::string name, int value) const;
	void setMat4(const std::string name, glm::mat4 value) const;
	void setVec3(const std::string name, glm::vec3 value) const;
	void setVec3(const std::string name, float v0, float v1, float v2) const;
	void setVec2(const std::string name, float v0, float v1) const;
	void setVec2(const std::string name, glm::vec2 v) const;
	void setMat3(const std::string name, glm::mat3 value) const;

	static std::string readFile(const char* path);	
};

