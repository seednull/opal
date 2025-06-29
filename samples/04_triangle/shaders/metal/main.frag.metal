#include <metal_math>

struct VertexToFragment
{
	float4 position [[position]];
	float4 color;
};

struct FragmentOutput
{
	float4 color [[color(0)]];
};

[[fragment]] FragmentOutput fragmentMain(VertexToFragment in [[stage_in]])
{
	FragmentOutput out;

	out.color.rgb = metal::pow(in.color.rgb, float3(1.0f / 2.2f));
	out.color.a =  in.color.a;

	return out;
}
