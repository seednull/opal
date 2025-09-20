// bindings

// ray payload
struct RayPayload
{
	float3 color;
};

[shader("miss")]
void rayMissMain(inout RayPayload payload : SV_RayPayload)
{
	payload.color = float3(0.4f, 0.4f, 0.4f);
}
