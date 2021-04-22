#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture0;
uniform float gamma;
uniform float exposure;

float offset = 1.0 / 300.0;

void main()
{		
	FragColor = vec4(texture(texture0, TexCoords).rg, 0.0, 1.0f);
}
