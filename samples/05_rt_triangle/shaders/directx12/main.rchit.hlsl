// bindings

// ray payload
struct RayPayload
{
	float3 color;
};

[shader("closesthit")]
void rayClosestHitMain(inout RayPayload payload : SV_RayPayload, in BuiltInTriangleIntersectionAttributes attribs : SV_IntersectionAttributes)
{
	float2 bary = attribs.barycentrics;
	
	float3 barycentric_coords = float3(1.0f - bary.x - bary.y, bary.x, bary.y);
	payload.color = pow(barycentric_coords, 1.0f / 2.2f);
}
