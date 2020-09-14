#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT
{
	vec3 Normal;
};

uniform mat4 view, model;

void main()
{
	mat3 normalMatrix = mat3(transpose(inverse(view * model)));
	//Normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
	Normal = normalize(normalMatrix * aNormal);
	gl_Position = view * model * vec4(aPos, 1.0);

	// projection matrix is applied at geometry shader, otherwise the lines generated will not
	// have correct w value
}
