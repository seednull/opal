struct VertexToFragment
{
	float4 data: TEXCOORD0;
};

static const float3 START_COLOR = float3(60.0f / 255.0f, 106.0f / 255.0f, 155.0f / 255.0f);
static const float3 END_COLOR = float3(57.0f / 255.0f, 218.0f / 255.0f, 155.0f / 255.0f);
static const float ATTENUATION_POWER = 0.01f;

float4 fragmentMain(VertexToFragment input): SV_Target
{
	float2 uv = input.data.xy * 0.5f + float2(0.5f, 0.5f);
	float lifetime = pow(saturate(input.data.z), 0.5f);

	float3 color = lerp(END_COLOR, START_COLOR, lifetime) * 0.02f;
	float attenuation = pow(saturate(1.0f - dot(input.data.xy, input.data.xy)), ATTENUATION_POWER);

	return float4(color * attenuation, 1.0f);
}
