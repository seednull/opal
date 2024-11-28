struct VertexOutput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
};

float4 main(VertexOutput input): SV_Target
{
	return input.color;
}
