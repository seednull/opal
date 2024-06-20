#version 450

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec2 in_uv;

layout(location = 0) out vec2 out_uv;

void main()
{
	gl_Position = vec4(in_position.xyz, 1.0);
	out_uv = in_uv;
}
