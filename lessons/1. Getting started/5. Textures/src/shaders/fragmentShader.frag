#version 330 core

out vec4 FragColor;

in vec4 outColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float ratio;

void main()
{		
	FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), ratio);
}