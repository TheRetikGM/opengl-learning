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
	vec2 TexCoord;
	vec3 TangFragPos;
	vec3 TangViewPos;
	vec3 TangLightPos;
} fs_in;		// vertex shader in

uniform Material material;
uniform Light light;
uniform bool normalMapping;
uniform bool parallaxMapping;
uniform float height_scale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir); // parameters in tangent space

void main()
{		
	vec3 lightDir = normalize(fs_in.TangLightPos - fs_in.TangFragPos);
	vec3 viewDir = normalize(fs_in.TangViewPos - fs_in.TangFragPos);
	vec2 texCoords = (parallaxMapping) ? ParallaxMapping(fs_in.TexCoord, viewDir) : fs_in.TexCoord;
	if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
		discard;
	vec3 color = texture(material.texture_diffuse0, texCoords).rgb;			
	vec3 normal;
	if (normalMapping) {
		normal = texture(material.texture_normal0, texCoords).rgb;
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