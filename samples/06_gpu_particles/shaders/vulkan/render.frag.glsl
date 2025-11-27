#version 450

layout(location = 0) in vec4 in_data;
layout(location = 0) out vec4 out_color;

vec3 START_COLOR = vec3(60.0f / 255.0f, 106.0f / 255.0f, 155.0f / 255.0f);
vec3 END_COLOR = vec3(57.0f / 255.0f, 218.0f / 255.0f, 155.0f / 255.0f);
float ATTENUATION_POWER = 0.01f;

void fragmentMain()
{
	vec2 uv = in_data.xy * 0.5f + vec2(0.5f);
	float lifetime = pow(in_data.z, 0.5f);

	vec3 color = mix(END_COLOR, START_COLOR, lifetime) * 0.02f;
	float attenuation = pow(clamp(1.0f - dot(in_data.xy, in_data.xy), 0.0f, 1.0f), ATTENUATION_POWER);

	out_color = vec4(color * attenuation, 1.0f);
}
