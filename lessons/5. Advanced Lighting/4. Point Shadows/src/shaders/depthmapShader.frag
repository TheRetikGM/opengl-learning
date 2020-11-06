#version 330 core

in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{	
	float fragDistance = length(FragPos.xyz - lightPos);
	fragDistance = fragDistance / far_plane;
	gl_FragDepth = fragDistance;
}