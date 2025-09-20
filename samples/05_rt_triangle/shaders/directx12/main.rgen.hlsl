#define DEG2RAD 0.0174532925f

// bindings
struct Camera
{
	float4 position;
	float4 direction;
};

cbuffer CameraBuffer : register(b0, space0)
{
	Camera camera;
};

RaytracingAccelerationStructure tlas : register(t1, space0);
RWTexture2D<float4> image : register(u2, space0);

// Ray payload
struct RayPayload
{
	float3 color;
};

[shader("raygeneration")]
void rayGenerationMain()
{
	// uv
	const uint2 launch_id = DispatchRaysIndex().xy;
	const uint2 launch_size = DispatchRaysDimensions().xy;
	const float aspect = float(launch_size.x) / launch_size.y;
	const float fov = 60.0f;
	const float focal_length = 1 / (2.0f * tan(fov * DEG2RAD * 0.5f));

	const float2 center = launch_id + 0.5f;
	float2 ndc = (center / launch_size) * 2.0f - 1.0f;
	ndc.x *= aspect;

	// gen
	float3 origin_ws = camera.position.xyz;

	float3 forward = camera.direction.xyz;
	float3 up = float3(0.0f, 0.0f, 1.0f);
	float3 right = normalize(cross(forward, up));
	up = normalize(cross(right, forward));

	float3 direction_ws = normalize(forward * focal_length + up * ndc.y + right * ndc.x);

	// trace
	RayPayload payload;
	payload.color = float3(0.0f, 0.0f, 0.0f);

	const float tmin = 0.001f;
	const float tmax = 10000.0f;

	const uint ray_type = 0;
	const uint num_ray_types = 0;
	const uint miss_index = 0;

	RayDesc desc;
	desc.Origin = origin_ws;
	desc.TMin = tmin;
	desc.Direction = direction_ws;
	desc.TMax = tmax;

	TraceRay(tlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, ray_type, num_ray_types, miss_index, desc, payload);

	// store
	image[launch_id] = float4(payload.color, 1.0f);
}
