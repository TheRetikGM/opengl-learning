#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

struct Light {
	vec3 position;
	vec3 diffuse, specular, ambient;
};

out VS_OUT
{
	vec3 Normal;
	vec2 TexCoord;
	vec3 TangFragPos;
	vec3 TangViewPos;
	vec3 TangLightPos;
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

	vec3 T = normalize(normalMatrix * aTangent);
	vec3 B = normalize(normalMatrix * aBitangent);
	vec3 N = normalize(normalMatrix * aNormal);
	mat3 TBN = (normalMapping) ? transpose(mat3(T, B, N)) : mat3(1.0);	

	TangLightPos = TBN * light.position;
	TangViewPos = TBN * viewPos;
	TangFragPos = TBN * vec3(model * vec4(aPos, 1.0));	// Fragment position in world space

	Normal = N;
	TexCoord = aTexCoord;	
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}