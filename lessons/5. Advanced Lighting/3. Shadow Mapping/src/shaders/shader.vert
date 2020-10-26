#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out VS_OUT
{
	vec3 Normal;
	vec2 TexCoord;
	vec3 FragPos;
	vec4 FragPosLightSpace;
};

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;

void main()
{
	Normal = normalMatrix * aNormal;
	TexCoord = aTexCoord;
	FragPos = vec3(model * vec4(aPos, 1.0));	// Fragment position in world space
	FragPosLightSpace = lightSpaceMatrix * model * vec4(aPos, 1.0);
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}