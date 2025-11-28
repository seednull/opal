struct VertexInput
{
	float4 position: LOCATION0;
	uint vertex_id: SV_VertexID;
};

struct VertexToFragment
{
	float4 data: TEXCOORD0;
	float4 position: SV_Position;
};

cbuffer Camera: register(b1, space0)
{
	float4x4 view;
	float4x4 projection;
};

static const float PARTICLE_SIZE = 0.005f;

static const float2 vertices[4] =
{
	float2(-1.0f, -1.0f),
	float2(-1.0f,  1.0f),
	float2( 1.0f, -1.0f),
	float2( 1.0f,  1.0f)
};

VertexToFragment vertexMain(VertexInput input)
{
	VertexToFragment output;

	float lifetime = pow(saturate(input.position.w), 0.1f);
	float size = PARTICLE_SIZE * lifetime;

	float2 offset = vertices[input.vertex_id];

	float4 view_position = mul(view, float4(input.position.xyz, 1.0f));
	view_position.x += offset.x * size;
	view_position.y += offset.y * size;

	output.position = mul(projection, view_position);
	output.data = float4(offset.x, offset.y, input.position.w, 0.0f);

	return output;
}
