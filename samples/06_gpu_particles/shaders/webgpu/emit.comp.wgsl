struct Application
{
	viewport: vec4f,
	time: f32,
	dt: f32
};

struct EmitterData
{
	num_free: atomic<i32>,
	num_particles: u32,
	num_triangles: u32,
	padding: u32,
	min_lifetime: f32,
	max_lifetime: f32,
	min_imass: f32,
	max_imass: f32
};

@group(0) @binding(0) var<uniform> application: Application;

@group(1) @binding(0) var<storage, read_write> positions: array<vec4f>;
@group(1) @binding(1) var<storage, read_write> velocities: array<vec4f>;
@group(1) @binding(2) var<storage, read_write> parameters: array<vec4f>;
@group(1) @binding(3) var<storage, read_write> free_indices: array<u32>;
@group(1) @binding(4) var<storage, read_write> emitter: EmitterData;
@group(1) @binding(5) var<storage, read_write> mesh_vertices: array<vec4f>;
@group(1) @binding(6) var<storage, read_write> mesh_indices: array<u32>;

const RANDOM_SCALE: vec4f = vec4f(443.897, 441.423, 0.0973, 0.1099);

@must_use
fn random3(p: vec3f) -> vec3f
{
	var v: vec3f = fract(p * RANDOM_SCALE.xyz);
	v += dot(v, v.yxz + vec3f(19.19));
	return fract((v.xxy + v.yzz) * v.zyx);
}

@must_use
fn randomPointOnTriangle(barycentric: vec2f, v0: vec4f, v1: vec4f, v2: vec4f) -> vec4f
{
	var b1: f32 = 1.0 - sqrt(barycentric.x);
	var b2: f32 = (1.0 - b1) * barycentric.y;

	var e1: vec4f = v1 - v0;
	var e2: vec4f = v2 - v0;

	return v0 + e1 * b1 + e2 * b2;
}

@must_use
fn popFreeParticleIndex() -> u32
{
	let index: i32 = atomicAdd(&emitter.num_free, -1);
	return free_indices[index];
}

struct ComputeInput
{
	@builtin(global_invocation_id) global_invocation_id: vec3u,
};

@compute @workgroup_size(256, 1, 1)
fn computeMain(input: ComputeInput)
{
	var free_index: u32 = input.global_invocation_id.x;
	var num_free: u32 = u32(atomicLoad(&emitter.num_free));
	if (free_index >= num_free)
	{
		return;
	}

	var triangle_index: u32 = free_index % emitter.num_triangles;
	var i0: u32 = mesh_indices[triangle_index * 3 + 0];
	var i1: u32 = mesh_indices[triangle_index * 3 + 1];
	var i2: u32 = mesh_indices[triangle_index * 3 + 2];

	var v0: vec4f = mesh_vertices[i0];
	var v1: vec4f = mesh_vertices[i1];
	var v2: vec4f = mesh_vertices[i2];

	var uv: vec3f = vec3f(f32(triangle_index), application.time, f32(free_index));
	var value: vec3f = random3(uv);

	var lifetime: f32 = mix(emitter.min_lifetime, emitter.max_lifetime, value.x);
	var imass: f32 = mix(emitter.min_imass, emitter.max_imass, value.x);

	var particle_index: u32 = popFreeParticleIndex();
	positions[particle_index] = randomPointOnTriangle(value.yz, v0, v1, v2);
	velocities[particle_index] = vec4f(0.0, 0.0, 0.0, 0.0);
	parameters[particle_index] = vec4f(lifetime, lifetime, imass, 0.0);
}
