#version 330 core

struct Light {
	vec3 position;
	vec3 diffuse, specular, ambient;
};

out vec4 FragColor;

in VS_OUT
{
	vec3 Normal;
	vec2 TexCoord;
	vec3 FragPos;
	vec4 FragPosLightSpace;
} vs_in;		// vertex shader in

uniform vec3 viewPos;
uniform Light light;
uniform sampler2D texture0;
uniform sampler2D depthMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir);

void main()
{
	vec3 color = texture(texture0, vs_in.TexCoord).rgb;	

	// make light direction towards the origin vec3(0.0) <-- we want directional light
	vec3 lightDir = normalize(light.position);//normalize(light.position - vs_in.FragPos);
	vec3 viewDir = normalize(viewPos - vs_in.FragPos);
	vec3 normal = normalize(vs_in.Normal);
	float fragDist = length(light.position - vs_in.FragPos);
	float attenuation = 1.0 / pow((1.0 + 0.07 * fragDist + 0.032 * fragDist * fragDist), 2.2);	
	//float attenuation = 1.0 / (fragDist * fragDist);

	// ambient
	vec3 ambient = light.ambient * color;
	// diffuse
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * light.diffuse * color;// * attenuation;
	// specular	
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfwayDir, normal), 0.0), 128.0);		
	vec3 specular = spec * light.specular;// * attenuation;	// assuming brite white light color	

	float shadow = ShadowCalculation(vs_in.FragPosLightSpace, normal, lightDir);
	FragColor.rgb = pow(ambient + (1.0 - shadow) * (diffuse + specular), vec3(1.0 / 2.2));
	FragColor.a = 1.0;
	//FragColor = vec4(ambient + diffuse + specular, 1.0);	
	// vec3 col = ambient + diffuse + specular;
	// FragColor = vec4(col, 1.0);
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;	// clip-space to screen-space (perspective division)
	projCoords = projCoords * 0.5 + 0.5;

	if (projCoords.z > 1.0)
		return 0.0;

	float closestDepth = texture(depthMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(depthMap, 0);
	int offset = 1;
	for (int x = -offset; x <= offset; ++x)
	{
		for (int y = -offset; y <= offset; ++y)
		{
			float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= pow(2 * offset + 1, 2);

	return shadow;

}