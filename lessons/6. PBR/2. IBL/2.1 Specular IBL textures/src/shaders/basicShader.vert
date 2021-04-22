#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 PVM;
out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = PVM * vec4(aPos, 1.0);
}