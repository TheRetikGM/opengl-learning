#version 330 core

struct Material
{
	sampler2D texture_diffuse0;
};

out vec4 FragColor;

in VS_OUT
{	
	vec2 TexCoord;
} fs_in;

uniform Material material;

void main()
{
	FragColor = texture(material.texture_diffuse0, fs_in.TexCoord);
}