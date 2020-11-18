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
} fs_in;		// vertex shader in

uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform bool normalMapping;
uniform bool parallaxMapping;
uniform float height_scale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir); // parameters in tangent space

void main()
{		
	vec3 lightDir = normalize(light.position - fs_in.FragPos);
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);		
	vec3 color = texture(material.texture_diffuse0, fs_in.TexCoords).rgb;			
	vec3 normal = normalize(fs_in.Normal);	
	
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
	vec3 specular = spec * light.specular; //* texture(material.texture_specular0, texCoords);

	FragColor.rgb = pow(ambient + (diffuse + specular) * attenuation, vec3(1.0 / 2.2));		
	FragColor.a = 1.0;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
	const float minLayers = 8.0;
	const float maxLayers = 32.0;
	float numLayers = mix(minLayers, maxLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));

	float layerDepth = 1.0 / numLayers;
	float currentLayerDepth = 0.0;

	vec2 P = viewDir.xy * height_scale;
	vec2 deltaTexCoords = P / numLayers;

	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(material.texture_height0, texCoords).r;

	while (currentDepthMapValue > currentLayerDepth)
	{
		currentTexCoords -= deltaTexCoords;
		currentDepthMapValue = texture(material.texture_height0, currentTexCoords).r;
		currentLayerDepth += layerDepth;
	}

	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(material.texture_height0, prevTexCoords).r - currentLayerDepth + layerDepth;

	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = mix(currentTexCoords, prevTexCoords, weight);

	return finalTexCoords;
}