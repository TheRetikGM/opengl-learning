#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 PVM;

void main()
{
	gl_Position = PVM * vec4(aPos.x, aPos.y, 0.0, 1.0);
	TexCoord = aTexCoord;
}