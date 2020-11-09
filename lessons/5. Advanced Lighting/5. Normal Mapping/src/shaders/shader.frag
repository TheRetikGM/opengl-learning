#version 330 core

struct Light {
	vec3 position;
	vec3 diffuse, specular, ambient;
};
struct Material {
	sampler2D texture_diffuse0;
	sampler2D texture_height0;
	sampler2D texture_specular0;
};

out vec4 FragColor;

in VS_OUT
{
	vec3 Normal;
	vec2 TexCoord;
	vec3 TangFragPos;
	vec3 TangViewPos;
	vec3 TangLightPos;
} fs_in;		// vertex shader in

uniform Material material;
uniform Light light;
uniform bool normalMapping;

void main()
{
	vec3 color = texture(material.texture_diffuse0, fs_in.TexCoord).rgb;	
	//vec3 color = vec3(1.0, 0.6, 0.34);

	// make light direction towards the origin vec3(0.0) <-- we want directional light
	vec3 lightDir = normalize(fs_in.TangLightPos - fs_in.TangFragPos);
	vec3 viewDir = normalize(fs_in.TangViewPos - fs_in.TangFragPos);
	vec3 normal;
	if (normalMapping) {
		normal = texture(material.texture_height0, fs_in.TexCoord).rgb;
		normal = normalize(normal * 2.0 - 1.0);
	}
	else {
		normal = fs_in.Normal;
	}
	
	float fragDist = length(fs_in.TangLightPos - fs_in.TangFragPos);
	float attenuation = 1.0 / (1.0 + 0.14 * fragDist + 0.07 * fragDist * fragDist);	
	//float attenuation = 1.0 / (fragDist * fragDist);

	// ambient
	vec3 ambient = light.ambient * color;
	// diffuse
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * light.diffuse * color;
	// specular	
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfwayDir, normal), 0.0), 128.0);		
	vec3 specular = spec * light.specular * texture(material.texture_specular0, fs_in.TexCoord);

	FragColor.rgb = pow(ambient + (diffuse + specular) * attenuation, vec3(1.0 / 2.2));
	FragColor.a = 1.0;
//	FragColor.rgb = pow(vec3(texture(depthCubemap, vs_in.FragPos - light.position).r), vec3(1.0/2.2));
//	FragColor.a = 1.0;
}