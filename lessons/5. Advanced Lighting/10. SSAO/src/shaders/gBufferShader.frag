#version 330 core

//out vec4 FragColor;
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

struct Material {
	sampler2D texture_diffuse0;
	sampler2D texture_normal0;
	sampler2D texture_height0;
	sampler2D texture_specular0;
};

in VS_OUT
{
	vec3 Normal;
	vec2 TexCoords;
	vec3 FragPos;
} fs_in;

uniform Material material;
uniform bool invertedNormals;

void main()
{
    //FragColor = texture(material.texture_diffuse0, fs_in.TexCoords);
    gPosition = fs_in.FragPos;
    gNormal = (invertedNormals) ? normalize(-fs_in.Normal) : normalize(fs_in.Normal);
    //gAlbedoSpec.rgb = texture(material.texture_diffuse0, fs_in.TexCoords).rgb;
	gAlbedoSpec.rgb = vec3(0.95);
    gAlbedoSpec.a = texture(material.texture_specular0, fs_in.TexCoords).r;
}