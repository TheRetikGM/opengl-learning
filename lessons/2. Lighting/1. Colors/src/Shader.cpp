#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char* vertexShaderSourcePath, const char* fragmentShaderSourcePath)
{	
	std::string v = readFile(vertexShaderSourcePath);
	std::string f = readFile(fragmentShaderSourcePath);
	const char *vertexSource = v.c_str();
	const char *fragmentSource = f.c_str();	

	int  success;
	char infoLog[512];

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);	
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "Shader.h: [ERROR] Vertex shader compilation failed!\n" << infoLog << std::endl;
	}

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "Shader.h: [ERROR] Fragment shader compilation failed!\n" << infoLog << std::endl;
	}

	Program = glCreateProgram();
	glAttachShader(Program, vertexShader);
	glAttachShader(Program, fragmentShader);
	glLinkProgram(Program);
	glGetProgramiv(Program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(Program, 512, NULL, infoLog);
		std::cerr << "[ERROR] Program linking failed\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}
Shader::Shader(std::string vertexShaderSourcePath, std::string fragmentShaderSourcePath)
{
	Shader(vertexShaderSourcePath.c_str(), fragmentShaderSourcePath.c_str());
}

std::string Shader::readFile(const char* path)
{
	std::string out;
	try
	{
		std::ifstream ifs(path);	// input file stream
		std::stringstream buffer;
		buffer << ifs.rdbuf();		
		out = buffer.str();		
	}
	catch (std::ifstream::failure e)
	{
		std::cerr << "[ERROR] Could not read/open file '" << path << "'\n";
	}
	return out;
}
void Shader::Use()
{
	glUseProgram(Program);
}
void Shader::setBool(const std::string name, bool value) const
{
	glUniform1i(glGetUniformLocation(Program, name.c_str()), value ? 1 : 0);
}
void Shader::setFloat(const std::string name, float value) const
{
	glUniform1f(glGetUniformLocation(Program, name.c_str()), value);
}
void Shader::setInt(const std::string name, int value) const
{
	glUniform1i(glGetUniformLocation(Program, name.c_str()), value);
}
void Shader::setMat4(const std::string name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(Program, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setVec3(const std::string name, glm::vec3 value) const
{
	setVec3(name, value.x, value.y, value.z);
}
void Shader::setVec3(const std::string name, float v0, float v1, float v2) const
{
	glUniform3f(glGetUniformLocation(Program, name.c_str()), v0, v1, v2);
}
