#version 330 core

struct Material {
	vec3 ambient_color;
	vec3 diffuse_color;
	vec3 specular_color;

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

in vec3 Normal;
in vec3 FragPos;

void main()
{
	vec3 lightDir = normalize(light.position - FragPos);	// direction towards light source
	vec3 normal = normalize(Normal);

	float ambientStrength = 0.1;
	vec3 ambient = light.ambient_intensity * material.ambient_color;

	float diff = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.diffuse_intensity * material.diffuse_color * diff;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normalize(viewPos - FragPos), reflectDir), 0.0), material.shininess);
	float specularStrength = 0.5;
	vec3 specular = light.specular_intensity * material.specular_color * spec;

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0f);	
}