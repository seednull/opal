#version 450

layout(location = 0) in vec4 in_color;
layout(location = 0) out vec4 out_color;

void fragmentMain()
{
	out_color.rgb = pow(in_color.rgb, vec3(1.0f / 2.2f));
	out_color.a = in_color.a;
}
