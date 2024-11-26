struct VertexInput
{
	float3 position: POSITION;
	float3 color: COLOR;
};

struct VertexShaderOutput
{
	float4 position: SV_Position;
	float4 color: COLOR;
};

VertexShaderOutput main(VertexInput input)
{
	VertexShaderOutput result;

	result.position = float4(input.position, 1.0f);
	result.color = float4(input.color, 1.0f);

	return result;
}
