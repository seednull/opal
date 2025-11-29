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

static const float4 RANDOM_SCALE = float4(443.897f, 441.423f, 0.0973f, 0.1099f);

float3 random3(float3 p)
{
	p = frac(p * RANDOM_SCALE.xyz);
	p += dot(p, p.yxz + 19.19f);
	return frac((p.xxy + p.yzz) * p.zyx);
}

uint pcg3d16(uint3 p)
{
	uint3 v = p * 1664525u + 1013904223u;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.x += v.y * v.z;
	return v.x;
}

float3 gradient3d(uint hash)
{
	float3 g = float3(uint3(0x80000u, 0x40000u, 0x20000u) & hash);
	return g * float3(1.0f / 0x40000, 1.0f / 0x20000, 1.0f / 0x10000) - 1.0f;
}

float simplexNoise3d(float x, float y, float z)
{
	float2 C = float2(1.0f / 6.0f, 1.0f / 3.0f);
	float4 D = float4(0.0f, 0.5f, 1.0f, 2.0f);

	// First corner
	float3 p = float3(x, y, z);
	float3 i = floor(p + dot(p, C.yyy));
	float3 x0 = p - i + dot(i, C.xxx);

	// Other corners
	float3 g = step(x0.yzx, x0.xyz);
	float3 l = 1.0f - g;
	float3 i1 = min(g.xyz, l.zxy);
	float3 i2 = max(g.xyz, l.zxy);

	float3 x1 = x0 - i1 + C.xxx;
	float3 x2 = x0 - i2 + C.yyy;
	float3 x3 = x0 - D.yyy;

	i += 32768.5f;
	uint hash0 = pcg3d16(uint3(i));
	uint hash1 = pcg3d16(uint3(i + i1));
	uint hash2 = pcg3d16(uint3(i + i2));
	uint hash3 = pcg3d16(uint3(i + 1.0f));

	float3 p0 = gradient3d(hash0);
	float3 p1 = gradient3d(hash1);
	float3 p2 = gradient3d(hash2);
	float3 p3 = gradient3d(hash3);

	// Mix final noise value
	float4 m = saturate(0.5f - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)));
	float4 mt = m * m;
	float4 m4 = mt * mt;

	return 62.6f * dot(m4, float4(dot(x0, p0), dot(x1, p1), dot(x2, p2), dot(x3, p3)));
}

float3 noise3d(float x, float y, float z)
{
	return float3(
		simplexNoise3d(x, y, z),
		simplexNoise3d(x, y + 1000.0f, z),
		simplexNoise3d(x, y, z + 1000.0f)
	);
}

float4 curlNoise3d(float4 p)
{
	float eps = 0.1f;

	float3 dfdx = noise3d(p.x + eps, p.y, p.z) - noise3d(p.x - eps, p.y, p.z);
	float3 dfdy = noise3d(p.x, p.y + eps, p.z) - noise3d(p.x, p.y - eps, p.z);
	float3 dfdz = noise3d(p.x, p.y, p.z + eps) - noise3d(p.x, p.y, p.z - eps);

	float3 curl = float3(
		dfdy.z - dfdz.y,
		dfdz.x - dfdx.z,
		dfdx.y - dfdy.x
	);

	return float4(normalize(curl), 0.0f);
}

void pushFreeParticleIndex(uint value)
{
	int index = 0;
	InterlockedAdd(emitter[0].num_free, 1, index);

	free_indices[index] = value;
}

[numthreads(256, 1, 1)]
void computeMain(uint3 global_invocation_id : SV_DispatchThreadID)
{
	uint particle_index = global_invocation_id.x;
	if (particle_index >= emitter[0].num_partices)
		return;

	float4 position = positions[particle_index];
	float4 velocity = velocities[particle_index];
	float4 parameter = parameters[particle_index];

	float4 offset = float4(time, time, time, 0.0f) * 0.1f;
	float scale = 0.5f;

	float4 acceleration = curlNoise3d(position * scale + offset) * parameter.z * 0.8f;

	parameter.x -= dt;
	float lifetime = clamp(parameter.x / parameter.y, 0.0f, 1.0f);

	velocity += acceleration * dt;
	position += velocity * dt;
	position.w = lifetime;

	if (parameter.x < 0.0f)
	{
		pushFreeParticleIndex(particle_index);
	}

	positions[particle_index] = position;
	velocities[particle_index] = velocity;
	parameters[particle_index] = parameter;
}
