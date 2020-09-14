#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 aModel;

out vec2 TexCoord;

uniform mat4 PV;

void main()
{
	TexCoord = aTexCoord;
	gl_Position = PV * aModel * vec4(aPos, 1.0);
}