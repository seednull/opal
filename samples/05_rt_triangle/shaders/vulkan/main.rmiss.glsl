#version 460
#extension GL_EXT_ray_tracing : enable

// bindings

// ray payload
layout(location = 0) rayPayloadInEXT vec3 payload_color;

void rayMissMain()
{
	payload_color = vec3(0.4f, 0.4f, 0.4f);
}
