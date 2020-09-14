#version 330

layout (location = 0) in vec3 aPosition;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};
uniform mat4 model;

void main()
{
	gl_Position = projection * view * model * vec4(aPosition, 1.0);	
}