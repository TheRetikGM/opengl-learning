#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

struct Light {
	vec3 position;
	vec3 diffuse, specular, ambient;
};

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
uniform vec3 viewPos;
uniform Light light;
uniform bool normalMapping;

void main()
{	
	FragPos =  vec3(model * vec4(aPos, 1.0));	// Fragment position in world space

	Normal = normalMatrix * aNormal;
	TexCoord = aTexCoord;	
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}