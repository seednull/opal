struct VertexInput
{
	float4 position: LOCATION0;
	float4 color: LOCATION1;
};

struct VertexOutput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
};

VertexOutput vertexMain(VertexInput input)
{
	VertexOutput result;

	result.position = float4(input.position.xyz, 1.0f);
	result.color = input.color;

	return result;
}
