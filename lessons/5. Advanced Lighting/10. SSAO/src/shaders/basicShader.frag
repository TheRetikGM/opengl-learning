#version 330 core

layout (location = 0) out vec4 FragColor;

uniform vec3 Color;

void main()
{
	//FragColor = vec4(Color.rgb, 1.0);
	// Gamma corrected color
	FragColor = vec4(pow(Color.rgb, vec3(1.0 / 2.2)), 1.0);
}