struct VertexOutput
{
	@builtin(position) position: vec4f,
	@location(0) color: vec4f
};

@fragment
fn main(input: VertexOutput) -> @location(0) vec4f
{
	return vec4f(pow(input.color.rgb, vec3f(1.0 / 2.2)), input.color.a);
}
