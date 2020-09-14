#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D quadTexture;

float offset = 1.0 / 300.0;

void main()
{
	// Grayscale
	//FragColor = texture(quadTexture, TexCoord);	
	//float avarage = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
	//FragColor = vec4(avarage, avarage, avarage, 1.0);
	
	vec2 offsets[9] = vec2[](
		vec2(-offset, offset),	 // top-left
		vec2(0.0, offset),		 // top-center
		vec2(offset, offset),	 // top-right
		vec2(-offset, 0.0),		 // center-right
		vec2(0.0, 0.0),			 // center-center
		vec2(offset, 0.0),		 // center-right
		vec2(-offset, -offset),	 // bottom-left
		vec2(0.0, -offset),		 // bottom-center
		vec2(offset, -offset)	 // bottom-right
	);

	float kernel[9] = float[](
		1, 1, 1,
		1, -8, 1,
		1, 1, 1
	);

	vec3 col = vec3(0.0);
	for (int i = 0; i < 9; i++)
	{
		col += vec3(texture(quadTexture, TexCoord.st + offsets[i])) * kernel[i];
	}
	FragColor = vec4(col, 1.0);
}
