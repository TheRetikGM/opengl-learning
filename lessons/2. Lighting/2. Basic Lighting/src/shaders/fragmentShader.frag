#version 330 core

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

in vec3 Normal;
in vec3 FragPos;

void main()
{
	vec3 lightDir = normalize(lightPos - FragPos);	// direction towards light source
	vec3 normal = normalize(Normal);

	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;	

	float diff = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = diff * objectColor;

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normalize(viewPos - FragPos), reflectDir), 0.0), 32.0);
	float specularStrength = 0.5;
	vec3 specular = specularStrength * spec * objectColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0f);	
}