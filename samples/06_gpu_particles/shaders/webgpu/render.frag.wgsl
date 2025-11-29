struct FragmentInput
{
	@builtin(position) position: vec4f,
	@location(0) data: vec4f
};

const START_COLOR: vec3f = vec3f(60.0 / 255.0, 106.0 / 255.0, 155.0 / 255.0);
const END_COLOR: vec3f = vec3f(57.0 / 255.0, 218.0 / 255.0, 155.0 / 255.0);
const ATTENUATION_POWER: f32 = 0.01;

@fragment
fn fragmentMain(input: FragmentInput) -> @location(0) vec4f
{
	var uv: vec2f = input.data.xy * 0.5 + vec2f(0.5);
	var lifetime: f32 = pow(input.data.z, 0.5);

	var color: vec3f = mix(END_COLOR, START_COLOR, lifetime) * 0.02;
	var attenuation: f32 = pow(clamp(1.0 - dot(input.data.xy, input.data.xy), 0.0, 1.0), ATTENUATION_POWER);

	return vec4f(color * attenuation, 1.0);
}
