struct VertexInput
{
	float4 position [[attribute(0)]];
	float4 color [[attribute(1)]];
};

struct VertexToFragment
{
	float4 position [[position]];
	float4 color;
};

[[vertex]] VertexToFragment vertexMain(VertexInput in [[stage_in]])
{
	VertexToFragment out;

	out.position = float4(in.position.xyz, 1.0f);
	out.color = in.color;

	return out;
}
