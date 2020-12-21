#version 330 core
layout (location = 0) out vec4 FragColor;

//out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture0;
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 tex_offset = 1.0 / textureSize(texture0, 0);
    vec3 result = texture(texture0, TexCoords).rgb * weight[0];
    if (horizontal) {
        for (int i = 0; i < 5; i++) {
            result += texture(texture0, TexCoords + vec2(i * tex_offset.x, 0.0)).rgb * weight[i];
            result += texture(texture0, TexCoords - vec2(i * tex_offset.x, 0.0)).rgb * weight[i];
        }
    } else {
        for (int i = 0; i < 5; i++) {
            result += texture(texture0, TexCoords + vec2(0.0, i * tex_offset.y)).rgb * weight[i];
            result += texture(texture0, TexCoords - vec2(0.0, i * tex_offset.y)).rgb * weight[i];
        }
    }
    FragColor = vec4(result.rgb, 1.0);    
    //FragColor = texture(texture0, TexCoords);
    //FragColor = vec4(0.0, 1.0, 1.0, 1.0);
}