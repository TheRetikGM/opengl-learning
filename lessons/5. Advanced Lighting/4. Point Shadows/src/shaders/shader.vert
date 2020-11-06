#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VS_OUT
{
	vec3 Normal;
	vec2 TexCoord;
	vec3 FragPos;
};

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;

void main()
{
	Normal = normalMatrix * aNormal;
	TexCoord = aTexCoord;
	FragPos = vec3(model * vec4(aPos, 1.0));	// Fragment position in world space
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}