#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform vec3 Color;
uniform sampler2D Texture;

void main()
{
	//FragColor = vec4(Color.rgb, 1.0);
	// Gamma corrected color	
	FragColor = vec4(pow(Color.rgb, vec3(1.0 / 2.2)), 1.0);
	//FragColor = vec4(pow(texture(Texture, TexCoords).rgb, vec3(1.0 / 2.2)), 1.0);
}