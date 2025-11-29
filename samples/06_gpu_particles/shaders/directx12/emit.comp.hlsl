cbuffer Application: register(b0, space0)
{
	float4 viewport;
	float time;
	float dt;
};

struct EmitterData
{
	int num_free;
	uint num_partices;
	uint num_triangles;
	uint padding;
	float min_lifetime;
	float max_lifetime;
	float min_imass;
	float max_imass;
};

RWStructuredBuffer<float4> positions: register(u0, space1);
RWStructuredBuffer<float4> velocities: register(u1, space1);
RWStructuredBuffer<float4> parameters: register(u2, space1);
RWStructuredBuffer<uint> free_indices: register(u3, space1);
RWStructuredBuffer<EmitterData> emitter: register(u4, space1);
RWStructuredBuffer<float4> mesh_vertices: register(u5, space1);
RWStructuredBuffer<uint> mesh_indices: register(u6, space1);

static const float4 RANDOM_SCALE = float4(443.897f, 441.423f, 0.0973f, 0.1099f);

float3 random3(float3 p)
{
	p = frac(p * RANDOM_SCALE.xyz);
	p += dot(p, p.yxz + 19.19f);
	return frac((p.xxy + p.yzz) * p.zyx);
}

float4 randomPointOnTriangle(float2 barycentric, float4 v0, float4 v1, float4 v2)
{
	float b1 = 1.0f - sqrt(barycentric.x);
	float b2 = (1.0f - b1) * barycentric.y;

	float4 e1 = v1 - v0;
	float4 e2 = v2 - v0;

	return v0 + e1 * b1 + e2 * b2;
}

uint popFreeParticleIndex()
{
	int index;
	InterlockedAdd(emitter[0].num_free, -1, index);

	return free_indices[index];
}

[numthreads(256, 1, 1)]
void computeMain(uint3 global_invocation_id : SV_DispatchThreadID)
{
	uint free_index = global_invocation_id.x;
	if (free_index >= uint(emitter[0].num_free))
		return;

	uint triangle_index = free_index % emitter[0].num_triangles;
	uint i0 = mesh_indices[triangle_index * 3u + 0u];
	uint i1 = mesh_indices[triangle_index * 3u + 1u];
	uint i2 = mesh_indices[triangle_index * 3u + 2u];

	float4 v0 = mesh_vertices[i0];
	float4 v1 = mesh_vertices[i1];
	float4 v2 = mesh_vertices[i2];

	float3 uv = float3(float(triangle_index), time, float(free_index));
	float3 value = random3(uv);

	float lifetime = lerp(emitter[0].min_lifetime, emitter[0].max_lifetime, value.x);
	float imass = lerp(emitter[0].min_imass, emitter[0].max_imass, value.x);

	uint particle_index = popFreeParticleIndex();
	positions[particle_index] = randomPointOnTriangle(value.yz, v0, v1, v2);
	velocities[particle_index] = float4(0.0f, 0.0f, 0.0f, 0.0f);
	parameters[particle_index] = float4(lifetime, lifetime, imass, 0.0f);
}
