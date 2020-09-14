#version 330 core

struct Material {	
	sampler2D diffuse_map;
	sampler2D specular_map;

	float shininess;
};
struct Light {
	vec3 position;

	vec3 ambient_intensity;
	vec3 diffuse_intensity;
	vec3 specular_intensity;
};

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform sampler2D emission_map;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

uniform float time;

void main()
{
	vec3 lightDir = normalize(light.position - FragPos);	// direction towards light source
	vec3 normal = normalize(Normal);
	
	vec3 ambient = light.ambient_intensity * vec3(texture(material.diffuse_map, TexCoord));

	float diff = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.diffuse_intensity * diff * vec3(texture(material.diffuse_map, TexCoord));

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normalize(viewPos - FragPos), reflectDir), 0.0), material.shininess);	
	vec3 specular = light.specular_intensity * spec * vec3(texture(material.specular_map, TexCoord));

	vec3 emission = vec3(0.0);
	if (vec3(texture(material.specular_map, TexCoord)) == vec3(0.0))
		emission = vec3(texture(emission_map, TexCoord + vec2(0.0, time)));

	vec3 result = ambient + diffuse + specular + emission;
	FragColor = vec4(result, 1.0f);	
}