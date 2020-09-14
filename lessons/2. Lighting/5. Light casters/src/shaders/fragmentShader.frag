#version 330 core

struct Material {	
	sampler2D diffuse_map;
	sampler2D specular_map;

	float shininess;
};
struct Light {
	vec3 position;
	vec3 direction;

	vec3 ambient_intensity;
	vec3 diffuse_intensity;
	vec3 specular_intensity;

	float constant;
	float linear;
	float quadratic;

	float cutOff;
	float outerCutOff;
};

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

void main()
{

//	vec3 lightDir = normalize(light.position - FragPos);	// direction towards light source
////	vec3 lightDir = normalize(-light.direction);
//	float frag_dist = length(light.position - FragPos);		// fragment distance from light source position
//	float attenuation = 1.0 / (light.constant + light.linear * frag_dist + light.quadratic * frag_dist * frag_dist);
//	vec3 normal = normalize(Normal);
//	
//	vec3 ambient = light.ambient_intensity * vec3(texture(material.diffuse_map, TexCoord));
//
//	float diff = max(dot(lightDir, normal), 0.0f);
//	vec3 diffuse = light.diffuse_intensity * diff * vec3(texture(material.diffuse_map, TexCoord));
//
//	vec3 reflectDir = reflect(-lightDir, normal);
//	float spec = pow(max(dot(normalize(viewPos - FragPos), reflectDir), 0.0), material.shininess);	
//	vec3 specular = light.specular_intensity * spec * vec3(texture(material.specular_map, TexCoord));	
//
//	vec3 result = (ambient + diffuse + specular) * attenuation;
//	FragColor = vec4(result, 1.0f);	
		

	vec3 lightDir = normalize(light.position - FragPos);	// direction towards light source
//	vec3 lightDir = normalize(-light.direction);	

	vec3 normal = normalize(Normal);

	vec3 ambient = light.ambient_intensity * vec3(texture(material.diffuse_map, TexCoord));

	float diff = max(dot(lightDir, normal), 0.0f);
	vec3 diffuse = light.diffuse_intensity * diff * vec3(texture(material.diffuse_map, TexCoord));

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(normalize(viewPos - FragPos), reflectDir), 0.0), material.shininess);	
	vec3 specular = light.specular_intensity * spec * vec3(texture(material.specular_map, TexCoord));	

	float dist    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist)); 

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	//float intensity = smoothstep(0.0, 1.0, (theta - light.outerCutOff) / epsilon);

	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0f);		
}