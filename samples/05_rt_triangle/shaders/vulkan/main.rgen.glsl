#version 460
#extension GL_EXT_ray_tracing : enable

#define DEG2RAD 0.0174532925f

// bindings
layout(set = 0, binding = 0) uniform Camera
{
	vec4 position;
	vec4 direction;
} camera;

layout(set = 0, binding = 1) uniform accelerationStructureEXT tlas;
layout(set = 0, binding = 2, rgba8) uniform image2D image;

// ray payload
layout(location = 0) rayPayloadEXT vec3 payload_color;

// entry
void rayGenerationMain()
{
	// uv
	const vec2 center = vec2(gl_LaunchIDEXT.xy) + vec2(0.5f);
	const float aspect = float(gl_LaunchSizeEXT.x) / gl_LaunchSizeEXT.y;
	const float fov = 60.0f;
	const float focal_length = 1 / (2.0f * tan(fov * DEG2RAD * 0.5f));

	vec2 ndc = center / vec2(gl_LaunchSizeEXT.xy) * 2.0f - vec2(1.0f);
	ndc.x *= aspect;


	// gen
	vec3 origin_ws = camera.position.xyz;

	vec3 forward = camera.direction.xyz;
	vec3 up = vec3(0.0f, 0.0f, 1.0f);
	vec3 right = normalize(cross(forward, up));
	up = normalize(cross(right, forward));

	vec3 direction_ws = normalize(forward * focal_length + up * ndc.y + right * ndc.x);

	// trace
	payload_color = vec3(0.0f);

	const float tmin = 0.001f;
	const float tmax = 10000.0f;

	const uint ray_type = 0;
	const uint num_ray_types = 0;
	const uint miss_index = 0;

	traceRayEXT(tlas, gl_RayFlagsOpaqueEXT, 0xff, ray_type, num_ray_types, miss_index, origin_ws, tmin, direction_ws, tmax, 0);

	// store
	imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(payload_color, 1.0));
}
