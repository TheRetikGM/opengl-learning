#version 330 core

struct Light {
	vec3 position;
	vec3 diffuse, specular, ambient;
};
struct Material {
	sampler2D texture_diffuse0;
	sampler2D texture_normal0;
	sampler2D texture_height0;
	sampler2D texture_specular0;
};

out vec4 FragColor;

in VS_OUT
{
	vec3 Normal;
	vec2 TexCoords;
	vec3 FragPos;
} fs_in;		// fragment shader in

uniform Material material;

uniform Light lights[16];
uniform vec3 viewPos;
uniform bool inverse_normals;
uniform int lights_num;
uniform float gamma;

vec3 CalcPointLight(Light light, vec3 color, vec3 normal, vec3 viewDir);

void main()
{				
	vec3 color = texture(material.texture_diffuse0, fs_in.TexCoords).rgb;			
	vec3 normal = normalize(fs_in.Normal) * ((inverse_normals) ? -1.0 : 1.0);	
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);		

	vec3 pointlight = vec3(0.0);
	for (int i = 0; i < lights_num; i++)	
		pointlight += CalcPointLight(lights[i], color, normal, viewDir);
	pointlight += 0.0005 * color; 	// ambient

	FragColor.rgb = pointlight;//pow(pointlight, vec3(1.0 / gamma));
	FragColor.a = 1.0;
}

vec3 CalcPointLight(Light light, vec3 color, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fs_in.FragPos);	

	float fragDist = length(light.position - fs_in.FragPos);
	//float attenuation = 1.0 / (1.0 + 0.14 * fragDist + 0.07 * fragDist * fragDist);	
	float attenuation = 1.0 / (fragDist * fragDist);

	// ambient
	vec3 ambient = light.ambient * color;
	// diffuse
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * light.diffuse * color;
	// specular	
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfwayDir, normal), 0.0), 128.0);		
	vec3 specular = spec * light.specular; //* texture(material.texture_specular0, texCoords);

	return (diffuse + specular) * attenuation;
}