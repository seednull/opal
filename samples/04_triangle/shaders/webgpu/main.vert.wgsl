struct VertexInput
{
	@builtin(vertex_index) index: u32,
	@location(0) position: vec4f,
	@location(1) color: vec4f
};

struct VertexOutput
{
	@builtin(position) position: vec4f,
	@location(0) color: vec4f
};

@vertex
fn vertexMain(input: VertexInput) -> VertexOutput
{
	var out: VertexOutput;

	out.position = vec4(input.position.xyz, 1.0);
	out.color = input.color;

	return out;
}
