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
} vs_in;		// vertex shader in

uniform vec3 viewPos;
uniform Light light;
uniform bool reverseNormal;
uniform sampler2D texture0;
uniform samplerCube depthCubemap;
uniform float far_plane;

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
); 

float ShadowCalculation(vec3 normal, vec3 fragPos);

void main()
{
	vec3 color = texture(texture0, vs_in.TexCoord).rgb;	
	//vec3 color = vec3(1.0, 0.6, 0.34);

	// make light direction towards the origin vec3(0.0) <-- we want directional light
	vec3 lightDir = normalize(light.position - vs_in.FragPos);
	vec3 viewDir = normalize(viewPos - vs_in.FragPos);
	vec3 normal = normalize(vs_in.Normal * ((reverseNormal) ? -1 : 1));
	float fragDist = length(light.position - vs_in.FragPos);
	float attenuation = 1.0 / (1.0 + 0.35 * fragDist + 0.44 * fragDist * fragDist);	
	//float attenuation = 1.0 / (fragDist * fragDist);

	// ambient
	vec3 ambient = light.ambient * color;
	// diffuse
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * light.diffuse * color;
	// specular	
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(halfwayDir, normal), 0.0), 128.0);		
	vec3 specular = spec * light.specular;	// assuming brite white light color	

	float shadow = ShadowCalculation(normal, vs_in.FragPos);
	FragColor.rgb = pow(ambient + (1.0 - shadow) * (diffuse + specular) * attenuation, vec3(1.0 / 2.2));
	FragColor.a = 1.0;
//	FragColor.rgb = vec3(texture(depthCubemap, vs_in.FragPos - light.position).r);
//	FragColor.a = 1.0;
}

float ShadowCalculation(vec3 normal, vec3 fragPos)
{
	vec3 LightFrag = fragPos - light.position;
	float currentDepth = length(LightFrag);
	float bias = 0.05; //max(0.05 * (1.0 - dot(normal, -LightFrag)), 0.005);
	float shadow = 0.0;
	int samples = 20;
	float viewDistance = length(fragPos - viewPos);
	float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
	//float diskRadius = 0.001;

	for (int i = 0; i < samples; i++)
	{
		float closestDepth = texture(depthCubemap, LightFrag + sampleOffsetDirections[i] * diskRadius).r;
		closestDepth *= far_plane;
		shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
	}
	shadow /= samples;

	return shadow;

}
