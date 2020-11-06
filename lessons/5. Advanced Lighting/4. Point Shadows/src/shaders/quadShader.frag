#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D texture0;
float offset = 1.0 / 300.0;

void main()
{
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
		0, 0, 0,
		0, 1, 0,
		0, 0, 0
	);

	vec3 col = vec3(0.0);
	for (int i = 0; i < 9; i++)
	{
		col += vec3(texture(texture0, TexCoord.st + offsets[i])) * kernel[i];
	}

	float gamma = 2.2;	
	//FragColor = vec4(pow(col, vec3(1.0 / gamma)), 1.0);	
	FragColor = vec4(col, 1.0f);
//	vec3 col = vec3(texture(texture0, TexCoord).r);
//	FragColor = vec4(pow(col, vec3(1.0 / 2.2)), 1.0);
}
