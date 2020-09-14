#pragma once
#include <glm/glm.hpp>

class DebugDrawer
{
public:		

	DebugDrawer();
	~DebugDrawer();

	void DrawCoordLines(glm::mat4 PVM);		// Projection * View * Model
	void ChangeCoordLineLength(float len);
private:
	float CoordLineLength;
	unsigned int Program;

	unsigned int VAO;
	unsigned int VBO;

	const char* vertexShaderSource;
	const char* fragmentShaderSource;

	float* coordLinesVertices;	

	void updateDebugLineVertices();
};