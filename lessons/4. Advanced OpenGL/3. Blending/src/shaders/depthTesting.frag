#version 330

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform float near;
uniform float far;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}


void main()
{
	FragColor = texture(texture1, TexCoord);
	//float depth = LinearizeDepth(gl_FragCoord.z) / far;
	//FragColor = vec4(vec3(depth), 1.0);
}