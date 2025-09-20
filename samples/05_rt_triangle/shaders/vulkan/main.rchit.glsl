#version 460
#extension GL_EXT_ray_tracing : enable

// bindings

// ray payload
layout(location = 0) rayPayloadInEXT vec3 payload_color;

// hit attributes
hitAttributeEXT vec2 attribs;

// entry
void rayClosestHitMain()
{
	const vec3 barycentric_coords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);

	payload_color = pow(barycentric_coords, vec3(1.0f / 2.2f));
}
