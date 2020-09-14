#version 330 core

struct Material {	
	sampler2D texture_diffuse0;
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_specular0;
	sampler2D texture_specular1;
	sampler2D texture_specular2;

	float shininess;
};
struct DirLight {
	vec3 direction;

	vec3 ambient_intensity;
	vec3 diffuse_intensity;
	vec3 specular_intensity;
};
struct PointLight {
	vec3 position;

	vec3 ambient_intensity;
	vec3 diffuse_intensity;
	vec3 specular_intensity;

	float constant;
	float linear;
	float quadratic;
};
struct SpotLight {
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

#define NR_POINT_LIGHTS 4
uniform SpotLight spotLight;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 fragPos);

void main()
{
	vec3 normal = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);	
	
//	vec3 result = CalcSpotLight(spotLight, normal, viewDir, FragPos);
	vec3 result = CalcPointLight(pointLights[0], normal, viewDir, FragPos);
	FragColor = vec4(result.rgb, 1.0);
}
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{	
	vec3 lightDir = normalize(-light.direction);		

	float diff = max(dot(lightDir, normal), 0.0f);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);	

	vec3 ambient = light.ambient_intensity * vec3(texture(material.texture_diffuse0, TexCoord));
	vec3 diffuse = light.diffuse_intensity * diff * vec3(texture(material.texture_diffuse0, TexCoord));
	vec3 specular = light.specular_intensity * spec * vec3(texture(material.texture_specular0, TexCoord));		

	return (ambient + diffuse + specular);
}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos)
{
	vec3 lightDir = normalize(light.position - fragPos);	// direction towards light source	

	float diff = max(dot(lightDir, normal), 0.0f);	

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);	

	vec3 ambient = light.ambient_intensity * vec3(texture(material.texture_diffuse0, TexCoord));
	vec3 diffuse = light.diffuse_intensity * diff * vec3(texture(material.texture_diffuse0, TexCoord));
	vec3 specular = light.specular_intensity * spec * vec3(texture(material.texture_specular0, TexCoord));	

	float frag_dist = length(light.position - fragPos);		// fragment distance from light source position
	float attenuation = 1.0 / (light.constant + light.linear * frag_dist + light.quadratic * frag_dist * frag_dist);	

	return ambient + (diffuse + specular) * attenuation;	
}
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 fragPos)
{
	vec3 lightDir = normalize(light.position - fragPos);	// direction towards light source		

	float diff = max(dot(lightDir, normal), 0.0f);	

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);	

	vec3 ambient = light.ambient_intensity * vec3(texture(material.texture_diffuse0, TexCoord));
	vec3 diffuse = light.diffuse_intensity * diff * vec3(texture(material.texture_diffuse0, TexCoord));
	vec3 specular = light.specular_intensity * spec * vec3(texture(material.texture_specular0, TexCoord));	

	float dist = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist)); 

	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	//float intensity = smoothstep(0.0, 1.0, (theta - light.outerCutOff) / epsilon);
	
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;

	return (ambient + diffuse + specular);
}