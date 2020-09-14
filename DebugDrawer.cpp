#include "DebugDrawer.h"
#include <GLAD/glad.h>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

DebugDrawer::DebugDrawer() : 
	vertexShaderSource(
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPos;\n"
		"layout(location = 1) in vec3 aColor;\n"
		"uniform mat4 PVM;\n"
		"out vec3 Color;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = PVM * vec4(aPos.xyz, 1.0);\n"
		"	Color = aColor;\n"
		"}\0"),
	fragmentShaderSource(
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 Color;\n"
		"void main()\n"
		"{\n"
		"	FragColor = vec4(Color, 1.0f);\n"
		"}\0"), 
	CoordLineLength(10.0f), coordLinesVertices(NULL)
{
	int  success;
	char infoLog[512];

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "DebugDrawer.h: [ERROR] Vertex shader compilation failed!\n" << infoLog << std::endl;
	}

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "DebugDrawer.h: [ERROR] Fragment shader compilation failed!\n" << infoLog << std::endl;
	}

	Program = glCreateProgram();
	glAttachShader(Program, vertexShader);
	glAttachShader(Program, fragmentShader);
	glLinkProgram(Program);
	glGetProgramiv(Program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(Program, 512, NULL, infoLog);
		std::cerr << "DebugDrawer.h: [ERROR] Program linking failed\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	updateDebugLineVertices();	
	
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}
void DebugDrawer::updateDebugLineVertices()
{
	if (coordLinesVertices != NULL)
		delete[] coordLinesVertices;

	coordLinesVertices = new float[72]{
		 -CoordLineLength, 0.0f, 0.0f,	0.5f, 0.0f, 0.0f,
		  0.0f, 0.0f, 0.0f,				0.5f, 0.0f, 0.0f,
		  0.0f, 0.0f, 0.0f,				1.0f, 0.0f, 0.0f,
		  CoordLineLength, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,  

		 0.0f, -CoordLineLength, 0.0f,	0.0f, 0.5f, 0.0f,
		 0.0f, 0.0f, 0.0f,				0.0f, 0.5f, 0.0f,
		 0.0f, 0.0f, 0.0f,				0.0f, 1.0f, 0.0f,
		 0.0f,  CoordLineLength, 0.0f,	0.0f, 1.0f, 0.0f,

		 0.0f, 0.0f, -CoordLineLength,	0.0f, 0.0f, 0.5f,
		 0.0f, 0.0f, 0.0f,				0.0f, 0.0f, 0.5f,
		 0.0f, 0.0f, 0.0f,				0.0f, 0.0f, 1.0f,
		 0.0f, 0.0f,  CoordLineLength,	0.0f, 0.0f, 1.0f
	};

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(float), coordLinesVertices, GL_STATIC_DRAW);
}
void DebugDrawer::ChangeCoordLineLength(float len)
{
	CoordLineLength = len;
	updateDebugLineVertices();
}
void DebugDrawer::DrawCoordLines(glm::mat4 PVM)
{
	glUseProgram(Program);
	glUniformMatrix4fv(glGetUniformLocation(Program, "PVM"), 1, GL_FALSE, glm::value_ptr(PVM));
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 12);
	glBindVertexArray(0);
}
DebugDrawer::~DebugDrawer()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(Program);
	delete[] coordLinesVertices;
}