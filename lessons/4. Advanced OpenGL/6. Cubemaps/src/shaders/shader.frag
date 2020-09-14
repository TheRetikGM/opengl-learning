#version 330

out vec4 FragColor;

in vec3 Position;
in vec3 Normal;

//uniform sampler2D texture1;
uniform samplerCube skybox;
uniform vec3 cameraPosition;

void main()
{
	float ratio = 1.0/1.52;
	vec3 I = normalize(Position - cameraPosition);
	//vec3 R = reflect(I, normalize(Normal));
	vec3 R = refract(I, normalize(Normal), ratio);
	FragColor = vec4(texture(skybox, R).rgb, 1.0);
}