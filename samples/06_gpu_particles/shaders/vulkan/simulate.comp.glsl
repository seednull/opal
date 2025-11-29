#version 450

layout(set = 0, binding = 0) uniform Application
{
	vec4 viewport;
	float time;
	float dt;
} application;

layout(std430, set = 1, binding = 0) buffer ParticlePositions
{
	vec4 data[];
} positions;

layout(std430, set = 1, binding = 1) buffer ParticleVelocities
{
	vec4 data[];
} velocities;

layout(std430, set = 1, binding = 2) buffer ParticleParameters
{
	vec4 data[];
} parameters;

layout(std430, set = 1, binding = 3) buffer FreeIndices
{
	uint data[];
} free_indices;

layout(std430, set = 1, binding = 4) buffer EmitterData
{
	int num_free;
	uint num_particles;
	uint num_triangles;
	uint padding;
	float min_lifetime;
	float max_lifetime;
	float min_imass;
	float max_imass;
} emitter;

const vec4 RANDOM_SCALE = vec4(443.897f, 441.423f, 0.0973f, 0.1099f);

vec3 random3(vec3 p)
{
	p = fract(p * RANDOM_SCALE.xyz);
	p += dot(p, p.yxz + 19.19f);
	return fract((p.xxy + p.yzz) * p.zyx);
}

uint pcg3d16(uvec3 p)
{
	uvec3 v = p * 1664525u + 1013904223u;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.x += v.y * v.z;
	return v.x;
}

vec3 gradient3d(uint hash)
{
	vec3 g = vec3(uvec3(0x80000, 0x40000, 0x20000) & hash);
	return g * vec3(1.0f / 0x40000, 1.0f / 0x20000, 1.0f / 0x10000) - 1.0f;
}

float simplexNoise3d(float x, float y, float z)
{
	vec2 C = vec2(1.0f / 6.0f, 1.0f / 3.0f);
	vec4 D = vec4(0.0f, 0.5f, 1.0f, 2.0f);

	// First corner
	vec3 p = vec3(x, y, z);
	vec3 i = floor(p + dot(p, C.yyy));
	vec3 x0 = p - i + dot(i, C.xxx);

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0f - g;
	vec3 i1 = min(g.xyz, l.zxy);
	vec3 i2 = max(g.xyz, l.zxy);

	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy;
	vec3 x3 = x0 - D.yyy;

	i += 32768.5f;
	uint hash0 = pcg3d16(uvec3(i));
	uint hash1 = pcg3d16(uvec3(i + i1));
	uint hash2 = pcg3d16(uvec3(i + i2));
	uint hash3 = pcg3d16(uvec3(i + 1.0f));

	vec3 p0 = gradient3d(hash0);
	vec3 p1 = gradient3d(hash1);
	vec3 p2 = gradient3d(hash2);
	vec3 p3 = gradient3d(hash3);

	// Mix final noise value
	vec4 m = clamp(0.5f - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0f, 1.0f);
	vec4 mt = m * m;
	vec4 m4 = mt * mt;

	return 62.6f * dot(m4, vec4(dot(x0, p0), dot(x1, p1), dot(x2, p2), dot(x3, p3)));
}

vec3 noise3d(float x, float y, float z)
{
	return vec3(
		simplexNoise3d(x, y, z),
		simplexNoise3d(x, y + 1000.0f, z),
		simplexNoise3d(x, y, z + 1000.0f)
	);
}

vec4 curlNoise3d(vec4 p)
{
	float eps = 0.1f;

	vec3 dfdx = noise3d(p.x + eps, p.y, p.z) - noise3d(p.x - eps, p.y, p.z);
	vec3 dfdy = noise3d(p.x, p.y + eps, p.z) - noise3d(p.x, p.y - eps, p.z);
	vec3 dfdz = noise3d(p.x, p.y, p.z + eps) - noise3d(p.x, p.y, p.z - eps);

	vec3 curl = vec3(
		dfdy.z - dfdz.y,
		dfdz.x - dfdx.z,
		dfdx.y - dfdy.x
	);

	return vec4(normalize(curl), 0.0f);
}

void pushFreeParticleIndex(uint value)
{
	uint index = atomicAdd(emitter.num_free, 1);
	free_indices.data[index] = value;
}

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void computeMain()
{
	uint particle_index = gl_GlobalInvocationID.x;
	if (particle_index >= emitter.num_particles)
		return;

	vec4 position = positions.data[particle_index];
	vec4 velocity = velocities.data[particle_index];
	vec4 parameter = parameters.data[particle_index];

	vec4 offset = vec4(application.time, application.time, application.time, 0.0f) * 0.1f;
	float scale = 0.5f;

	vec4 acceleration = curlNoise3d(position * scale + offset) * parameter.z * 0.8f;

	parameter.x -= application.dt;
	float lifetime = clamp(parameter.x / parameter.y, 0.0f, 1.0f);

	velocity += acceleration * application.dt;
	position += velocity * application.dt;
	position.w = lifetime;

	if (parameter.x < 0.0f)
	{
		pushFreeParticleIndex(particle_index);
	}

	positions.data[particle_index] = position;
	velocities.data[particle_index] = velocity;
	parameters.data[particle_index] = parameter;
}
