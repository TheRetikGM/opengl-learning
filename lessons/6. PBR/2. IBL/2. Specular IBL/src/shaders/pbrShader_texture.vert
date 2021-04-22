#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

void main()
{
	vec4 wpos = model * vec4(aPos, 1.0);
	TexCoords = aTexCoords;
	Normal = normalMatrix * aNormal;	
	WorldPos = wpos.xyz;
	gl_Position = projection * view * wpos;
}