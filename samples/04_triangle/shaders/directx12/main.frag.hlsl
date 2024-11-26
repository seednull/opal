struct PixelShaderInput
{
	float4 color: COLOR;
};

float4 main(PixelShaderInput input): SV_Target
{
	return input.color;
}
