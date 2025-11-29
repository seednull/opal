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

const RANDOM_SCALE: vec4f = vec4f(443.897, 441.423, 0.0973, 0.1099);

@must_use
fn random3(p: vec3f) -> vec3f
{
	var v: vec3f = fract(p * RANDOM_SCALE.xyz);
	v += dot(v, v.yxz + vec3f(19.19));
	return fract((v.xxy + v.yzz) * v.zyx);
}

@must_use
fn pcg3d16(p: vec3u) -> u32
{
	var v: vec3u = p * 1664525u + vec3u(1013904223u);
	v.x += v.y*v.z; v.y += v.z*v.x; v.z += v.x*v.y;
	v.x += v.y*v.z;
	return v.x;
}

@must_use
fn gradient3d(hash: u32) -> vec3f
{
	var g: vec3f = vec3f(vec3u(0x80000, 0x40000, 0x20000) & vec3u(hash));
	return g * vec3f(1.0 / 0x40000, 1.0 / 0x20000, 1.0 / 0x10000) - vec3f(1.0);
}

@must_use
fn simplexNoise3d(x: f32, y: f32, z: f32) -> f32
{
	const C: vec2f = vec2f(1.0 / 6.0, 1.0 / 3.0);
	const D: vec4f = vec4f(0.0, 0.5f, 1.0, 2.0);

	// First corner
	var p: vec3f = vec3f(x, y, z);
	var i: vec3f = floor(p + dot(p, C.yyy));
	var x0: vec3f = p - i + dot(i, C.xxx);

	// Other corners
	var g: vec3f = step(x0.yzx, x0.xyz);
	var l: vec3f = vec3f(1.0) - g;
	var i1: vec3f = min(g.xyz, l.zxy);
	var i2: vec3f = max(g.xyz, l.zxy);

	var x1: vec3f = x0 - i1 + C.xxx;
	var x2: vec3f = x0 - i2 + C.yyy;
	var x3: vec3f = x0 - D.yyy;

	i += vec3f(32768.5);
	var hash0: u32 = pcg3d16(vec3u(i));
	var hash1: u32 = pcg3d16(vec3u(i + i1));
	var hash2: u32 = pcg3d16(vec3u(i + i2));
	var hash3: u32 = pcg3d16(vec3u(i + vec3f(1.0)));

	var p0: vec3f = gradient3d(hash0);
	var p1: vec3f = gradient3d(hash1);
	var p2: vec3f = gradient3d(hash2);
	var p3: vec3f = gradient3d(hash3);

	// Mix final noise value
	var m: vec4f = clamp(vec4f(0.5f) - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), vec4f(0.0f), vec4f(1.0f));
	var mt: vec4f = m * m;
	var m4: vec4f = mt * mt;
	return 62.6f * dot(m4, vec4f(dot(x0, p0), dot(x1, p1), dot(x2, p2), dot(x3, p3)));
}

@must_use
fn noise3d(x: f32, y: f32, z: f32) -> vec3f
{
	return vec3f(
		simplexNoise3d(x, y, z),
		simplexNoise3d(x, y + 1000.0, z),
		simplexNoise3d(x, y, z + 1000.0)
	);
}

@must_use
fn curlNoise3d(p: vec4f) -> vec4f
{
	const eps: f32 = 0.1;

	var dfdx: vec3f = noise3d(p.x + eps, p.y, p.z) - noise3d(p.x - eps, p.y, p.z);
	var dfdy: vec3f = noise3d(p.x, p.y + eps, p.z) - noise3d(p.x, p.y - eps, p.z);
	var dfdz: vec3f = noise3d(p.x, p.y, p.z + eps) - noise3d(p.x, p.y, p.z - eps);

	var curl: vec3f = vec3f(
		dfdy.z - dfdz.y,
		dfdz.x - dfdx.z,
		dfdx.y - dfdy.x
	);

	return vec4f(normalize(curl), 0.0);
}

fn pushFreeParticleIndex(value: u32)
{
	let index: i32 = atomicAdd(&emitter.num_free, 1);
	free_indices[index] = value;
}

struct ComputeInput
{
	@builtin(global_invocation_id) global_invocation_id: vec3u,
};

@compute @workgroup_size(256, 1, 1)
fn computeMain(input: ComputeInput)
{
	var particle_index: u32 = input.global_invocation_id.x;
	if (particle_index >= emitter.num_particles)
	{
		return;
	}

	var position: vec4f = positions[particle_index];
	var velocity: vec4f = velocities[particle_index];
	var parameter: vec4f = parameters[particle_index];

	var offset: vec4f = vec4f(application.time, application.time, application.time, 0.0) * 0.1;
	const scale: f32 = 0.5f;

	var acceleration: vec4f = curlNoise3d(position * scale + offset) * parameter.z * 0.8;

	parameter.x -= application.dt;
	var lifetime: f32 = clamp(parameter.x / parameter.y, 0.0, 1.0);

	velocity += acceleration * application.dt;
	position += velocity * application.dt;
	position.w = lifetime;

	if (parameter.x < 0.0)
	{
		pushFreeParticleIndex(particle_index);
	}

	positions[particle_index] = position;
	velocities[particle_index] = velocity;
	parameters[particle_index] = parameter;
}
