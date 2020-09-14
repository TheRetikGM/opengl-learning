#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT
{
	vec3 Normal;
} gs_in[];

const float MAGNITUDE = 0.4;

uniform mat4 projection;

vec3 GetNormal()
{
	vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(a, b));
}
void GenLine(int index)
{
	gl_Position = projection * gl_in[index].gl_Position;
	EmitVertex();
	gl_Position = projection * (gl_in[index].gl_Position + vec4(gs_in[index].Normal, 0.0) * MAGNITUDE);
	EmitVertex();
	EndPrimitive();
}

void main()
{
	GenLine(0);
	GenLine(1);
	GenLine(2);
}