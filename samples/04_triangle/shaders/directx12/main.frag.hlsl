struct VertexOutput
{
	float4 position: SV_Position;
	float4 color: COLOR0;
};

float4 fragmentMain(VertexOutput input): SV_Target
{
	return float4(pow(input.color.rgb, 1.0f / 2.2f), input.color.a);
}
