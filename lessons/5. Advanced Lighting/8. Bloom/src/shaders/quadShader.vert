#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 PV, model;

out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoord;
	gl_Position = vec4(aPos, 1.0);
}