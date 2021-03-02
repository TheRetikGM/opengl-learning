#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;


out VS_OUT
{
	vec3 Normal;
	vec2 TexCoords;
	vec3 FragPos;
};

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;

void main()
{	
	// World space
	vec4 viewPos = view * model * vec4(aPos.xyz, 1.0);
	FragPos = viewPos.xyz;	
	Normal = normalMatrix * aNormal;

	TexCoords = aTexCoords;	
	// Screen space
	gl_Position = projection * viewPos;
}