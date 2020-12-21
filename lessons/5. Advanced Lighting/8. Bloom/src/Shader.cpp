#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "DebugColors.h"

Shader::Shader(const char* vertexShaderSourcePath, const char* fragmentShaderSourcePath, const char* geometryShaderSourcePath, bool directSource)
{	
	const char* vertexSource;
	const char* fragmentSource;
	const char* geometrySource;
	std::string v, f, g;
	if (!directSource)
	{
		v = readFile(vertexShaderSourcePath);
		f = readFile(fragmentShaderSourcePath);

		if (v == "" || f == "") {
			this->Program = 0;
			return;
		}
		vertexSource = v.c_str();
		fragmentSource = f.c_str();
		
		if (geometryShaderSourcePath != NULL)
		{
			g = readFile(geometryShaderSourcePath);

			if (g == "") {
				this->Program = 0;
				return;
			}
			geometrySource = g.c_str();
		}
		else
			geometrySource = NULL;
	}
	else
	{
		vertexSource = vertexShaderSourcePath;
		fragmentSource = fragmentShaderSourcePath;
		geometrySource = geometryShaderSourcePath;
	}

	int  success;
	char infoLog[512];

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);	
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{		
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "Shader.h: " DC_WARNING " Vertex shader compilation failed!\n";
		std::cerr << "shader.h: Path: " << vertexShaderSourcePath << '\n' << infoLog << std::endl;
		this->Program = 0;
		return;
	}

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "Shader.h: " DC_WARNING " Fragment shader compilation failed!\n";
		std::cerr << "shader.h: Path: " << fragmentShaderSourcePath << '\n' << infoLog << std::endl;
		this->Program = 0;
		return;
	}

	unsigned int geometryShader;
	if (geometrySource)
	{
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometrySource, NULL);
		glCompileShader(geometryShader);
		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
			std::cerr << "Shader.h: " DC_WARNING " Geometry shader compilation failed!\n";
			std::cerr << "Shader.h: Path: " << geometryShaderSourcePath;
			std::cerr << std::endl << infoLog << std::endl;
			this->Program = 0;
			return;
		}
	}

	Program = glCreateProgram();
	glAttachShader(Program, vertexShader);
	glAttachShader(Program, fragmentShader);
	if (geometrySource)
		glAttachShader(Program, geometryShader);
	glLinkProgram(Program);
	glGetProgramiv(Program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(Program, 512, NULL, infoLog);
		std::cerr << "shader.h: " DC_ERROR " Program linking failed\n" << infoLog << std::endl;
		this->Program = 0;
		return;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}
Shader::Shader(std::string vertexShaderSourcePath,
	std::string fragmentShaderSourcePath,
	std::string geometryShaderSource,
	bool directSource) : Shader(vertexShaderSourcePath.c_str(), fragmentShaderSourcePath.c_str(), geometryShaderSource.c_str(), directSource)
{	
}
Shader::~Shader()
{
	glDeleteProgram(Program);
}

std::string Shader::readFile(const char* path)
{
	std::ifstream ifs(path);	// input file stream

	if (ifs.fail())
	{
		std::cout << "shader.h: " DC_ERROR " Could not open file '" << path << "'\n";
		return "";
	}

	std::stringstream buffer;
	buffer << ifs.rdbuf();		
	std::string out = buffer.str();	
	ifs.close();

	return out;
}
void Shader::Use() const
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
void Shader::setMat3(const std::string name, glm::mat3 value) const
{
	glUniformMatrix3fv(glGetUniformLocation(Program, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setVec2(const std::string name, float v0, float v1) const
{
	glUniform2f(glGetUniformLocation(Program, name.c_str()), v0, v1);
}
void Shader::setVec2(const std::string name, glm::vec2 v) const
{
	setVec2(name, v.x, v.y);
}
