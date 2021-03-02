#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

struct GBuffer
{
    sampler2D texture_position;
    sampler2D texture_normal;
    sampler2D texture_albedo_spec;
};
struct Light {
	vec3 position;
	vec3 diffuse, specular, ambient;
};

uniform Light lights[32];
uniform int lights_num;
uniform GBuffer gBuffer;
uniform vec3 viewPos;
uniform sampler2D texture_occlusion;

vec3 CalcPointLight(Light light, vec3 color, vec3 normal, vec3 viewDir, vec3 fragPos);

void main()
{
    vec3 color = texture(gBuffer.texture_albedo_spec, TexCoords).rgb;
    vec3 normal = texture(gBuffer.texture_normal, TexCoords).rgb;
    vec3 fragPos = texture(gBuffer.texture_position, TexCoords).rgb;
	float occlusion = texture(texture_occlusion, TexCoords).r;
    vec3 viewDir = normalize(-fragPos);

    vec3 pointLight = vec3(0.0);
    for (int i = 0; i < lights_num; i++)
        pointLight += CalcPointLight(lights[i], color, normal, viewDir, fragPos);
    pointLight += lights[0].ambient * color * occlusion; // ambient	

    FragColor = vec4(pow(pointLight, vec3(1.0/2.2)), 1.0);	
}

vec3 CalcPointLight(Light light, vec3 color, vec3 normal, vec3 viewDir, vec3 fragPos)
{
	vec3 lightDir = normalize(light.position - fragPos);	

	float fragDist = length(light.position - fragPos);
	float attenuation = 1.0 / (1.0 + 0.09 * fragDist + 0.032 * fragDist * fragDist);	
	//float attenuation = 1.0 / (fragDist * fragDist);

	// ambient
	// vec3 ambient = light.ambient * color;
	// diffuse
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * light.diffuse * color;
	// specular	
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfwayDir, normal), 0.0), 128.0);		
	vec3 specular = spec * light.specular * vec3(texture(gBuffer.texture_albedo_spec, TexCoords).a);

	return (diffuse + specular) * attenuation;
}