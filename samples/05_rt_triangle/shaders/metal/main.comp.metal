#include <metal_stdlib>
#include <metal_raytracing>

using namespace metal;
using namespace raytracing;

constant float DEG2RAD = 0.0174532925f;

struct Camera
{
	float4 position;
	float4 direction;
};

struct RaytraceBindings
{
	constant Camera& camera [[id(0)]];
	instance_acceleration_structure tlas [[id(1)]];
	texture2d<float, access::write> image [[id(2)]];
};

kernel void computeMain(
	constant RaytraceBindings &bindings [[buffer(0)]],
	uint2 thread_position_in_grid [[thread_position_in_grid]],
	uint2 grid_size [[threads_per_grid]]
)
{
	// uv
	float2 center = float2(thread_position_in_grid) + float2(0.5f);
	float aspect = float(grid_size.x) / float(grid_size.y);
	float fov = 60.0f;
	float focal_length = 1.0f / (2.0f * tan(fov * DEG2RAD * 0.5f));
	
	float2 ndc = center / float2(grid_size) * 2.0f - float2(1.0f);
	ndc.x *= aspect;

	// gen
	float3 origin_ws = bindings.camera.position.xyz;

	float3 forward = normalize(bindings.camera.direction.xyz);
	float3 up = float3(0.0f, 0.0f, 1.0f);
	float3 right = normalize(cross(forward, up));
	up = normalize(cross(right, forward));

	float3 direction_ws = normalize(forward * focal_length + up * ndc.y + right * ndc.x);

	// trace
	ray ray;
	ray.origin = origin_ws;
	ray.direction = direction_ws;
	ray.min_distance = 0.001f;
	ray.max_distance = 10000.0f;

	intersector<instancing, triangle_data> intersector;
	intersection_result<instancing, triangle_data> intersection;

	intersector.accept_any_intersection(false);
	intersection = intersector.intersect(ray, bindings.tlas);

	float3 color = float3(0.0f);
	
	if (intersection.type != intersection_type::none)
	{
		float2 attribs = intersection.triangle_barycentric_coord;
		float3 barycentric_coords = float3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
		color = pow(barycentric_coords, float3(1.0f / 2.2f));
	} else
	{
		color = float3(0.4f, 0.4f, 0.4f);
	}

	// store
	bindings.image.write(float4(color, 1.0f), thread_position_in_grid);
}