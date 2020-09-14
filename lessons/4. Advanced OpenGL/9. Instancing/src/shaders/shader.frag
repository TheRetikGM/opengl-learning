#version 330 core

struct Material
{
	sampler2D texture_diffuse0;
};

out vec4 FragColor;
in vec2 TexCoord;

uniform Material material;

void main()
{
	FragColor = texture(material.texture_diffuse0, TexCoord);
}