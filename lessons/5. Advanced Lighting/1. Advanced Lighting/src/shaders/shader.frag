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
uniform sampler2D texture0;
uniform bool blinn_phong;

void main()
{
	vec3 color = texture(texture0, vs_in.TexCoord).rgb;

	vec3 lightDir = normalize(light.position - vs_in.FragPos);
	vec3 viewDir = normalize(viewPos - vs_in.FragPos);
	vec3 normal = normalize(vs_in.Normal);
	float fragDist = length(light.position - vs_in.FragPos);
	//float attenuation = 1.0 / (1.0 + 0.07 * fragDist + 0.032 * fragDist * fragDist);	
	float attenuation = 1.0 / (fragDist);

	// ambient
	vec3 ambient = light.ambient * color;
	// diffuse
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * light.diffuse * color * attenuation;
	// specular
	float spec = 0.0;
	if (blinn_phong) {
		vec3 halfwayDir = normalize(lightDir + viewDir);
		spec = pow(max(dot(halfwayDir, normal), 0.0), 32.0);
	} else {
		vec3 reflectDir = normalize(reflect(-lightDir, normal));
		spec = pow(max(dot(reflectDir, viewDir), 0.0), 8.0);
	}
	vec3 specular = spec * light.specular * attenuation;	// assuming brite white light color	

	FragColor = vec4(ambient + diffuse + specular, 1.0);
}
