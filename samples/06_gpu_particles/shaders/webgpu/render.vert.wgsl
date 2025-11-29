struct VertexInput
{
	@builtin(vertex_index) index: u32,
	@location(0) position: vec4f
};

struct VertexOutput
{
	@builtin(position) position: vec4f,
	@location(0) data: vec4f
};

struct Camera
{
	view: mat4x4f,
	projection: mat4x4f
};

const PARTICLE_SIZE: f32 = 0.005;

const VERTICES = array(
	vec2f(-1.0f, -1.0f),
	vec2f(-1.0f,  1.0f),
	vec2f( 1.0f, -1.0f),
	vec2f( 1.0f,  1.0f),
);

@group(0) @binding(1) var<uniform> camera: Camera;

@vertex
fn vertexMain(input: VertexInput) -> VertexOutput
{
	var lifetime: f32 = pow(clamp(input.position.w, 0.0, 1.0), 0.1);
	var size: f32 = PARTICLE_SIZE * lifetime;

	var offset: vec2f = VERTICES[input.index];

	var view_position: vec4f = camera.view * vec4f(input.position.xyz, 1.0);
	view_position.x += offset.x * size;
	view_position.y += offset.y * size;

	var out: VertexOutput;
	out.position = camera.projection * view_position; 
	out.data = vec4f(offset.x, offset.y, input.position.w, 0.0);

	return out;
}
