#version 330 core
out float FragColor;	// output only red component

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
const int kernelSize = 64;
uniform mat4 projection;

// Tile noise texture over screen, based on screen dimensions divided by noise size.
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0);

void main()
{
	vec3 fragPos = texture(gPosition, TexCoords).xyz;
	vec3 normal = normalize(texture(gNormal, TexCoords).xyz);
	vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	float radius = 0.5;
	float bias = 0.025;
	for (int i = 0; i < kernelSize; ++i)
	{
		// Get sample position.
		vec3 samplePos = TBN * samples[i];	// Transform sample from tangent space to view space.
		samplePos = fragPos + samplePos * radius;

		// Transform samplePos to screen space
		vec4 offset = vec4(samplePos, 1.0);
		offset = projection * offset;			// From view to clip-space.
		offset.xyz /= offset.w;					// Perspective division.
		offset.xyz = offset.xyz * 0.5 + 0.5;	// Transform to range 0.0 - 1.0.

		float sampleDepth = texture(gPosition, offset.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = 1.0 - (occlusion / kernelSize);
	FragColor = occlusion;
}