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
	int num_particles;
	int num_triangles;
	int padding;
	float min_lifetime;
	float max_lifetime;
	float min_imass;
	float max_imass;
} emitter;

layout(std430, set = 1, binding = 5) buffer MeshVertices
{
	vec4 data[];
} mesh_vertices;

layout(std430, set = 1, binding = 6) buffer MeshIndices
{
	uint data[];
} mesh_indices;

const vec4 RANDOM_SCALE = vec4(443.897f, 441.423f, 0.0973f, 0.1099f);

vec2 random2(vec2 p)
{
	vec3 p3 = fract(p.xyx * RANDOM_SCALE.xyz);
	p3 += dot(p3, p3.yzx + 19.19f);
	return fract((p3.xx + p3.yz) * p3.zy);
}

vec3 random3(vec3 p)
{
	p = fract(p * RANDOM_SCALE.xyz);
	p += dot(p, p.yxz + 19.19f);
	return fract((p.xxy + p.yzz) * p.zyx);
}

vec4 randomPointOnTriangle(vec2 barycentric, vec4 v0, vec4 v1, vec4 v2)
{
	float b1 = 1.0f - sqrt(barycentric.x);
	float b2 = (1.0f - b1) * barycentric.y;

	vec4 e1 = v1 - v0;
	vec4 e2 = v2 - v0;

	return v0 + e1 * b1 + e2 * b2;
}

uint popFreeParticleIndex()
{
	uint index = atomicAdd(emitter.num_free, -1);
	return free_indices.data[index];
}

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void computeMain()
{
	uint free_index = gl_GlobalInvocationID.x;
	if (free_index >= emitter.num_free)
		return;

	uint triangle_index = free_index % uint(emitter.num_triangles);
	uint i0 = mesh_indices.data[triangle_index * 3 + 0];
	uint i1 = mesh_indices.data[triangle_index * 3 + 1];
	uint i2 = mesh_indices.data[triangle_index * 3 + 2];

	vec4 v0 = mesh_vertices.data[i0];
	vec4 v1 = mesh_vertices.data[i1];
	vec4 v2 = mesh_vertices.data[i2];

	vec3 uv = vec3(float(triangle_index), application.time, float(free_index));
	vec3 value = random3(uv);

	float lifetime = mix(emitter.min_lifetime, emitter.max_lifetime, value.x);
	float imass = mix(emitter.min_imass, emitter.max_imass, value.x);

	uint particle_index = popFreeParticleIndex();
	positions.data[particle_index] = randomPointOnTriangle(value.yz, v0, v1, v2);
	velocities.data[particle_index] = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	parameters.data[particle_index] = vec4(lifetime, lifetime, imass, 0.0f);
}
