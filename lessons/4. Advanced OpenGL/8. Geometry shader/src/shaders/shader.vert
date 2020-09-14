#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VS_OUT
{	
	vec2 TexCoord;
};

uniform mat4 PVM;

void main()
{
	TexCoord = aTexCoord;
	gl_Position = PVM * vec4(aPos, 1.0);
}