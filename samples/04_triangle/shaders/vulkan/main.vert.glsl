#version 450

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_color;

layout(location = 0) out vec4 out_color;

void vertexMain()
{
	gl_Position = vec4(in_position.xyz, 1.0);
	out_color = in_color;
}
