#version 330

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normal;
out vec3 Position;

void main()
{
	Normal = mat3(transpose(inverse(model))) * aNormal;
	Position = vec3(model * vec4(aPosition, 1.0));
	gl_Position = projection * view * vec4(Position, 1.0);	
}