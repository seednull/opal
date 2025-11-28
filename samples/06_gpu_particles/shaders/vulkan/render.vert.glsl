#version 450

layout(location = 0) in vec4 in_position;

layout(set = 0, binding = 1) uniform Camera
{
	mat4 view;
	mat4 projection;
} camera;

layout(location = 0) out vec4 out_data;

const float PARTICLE_SIZE = 0.005f;

const vec2 vertices[4] =
{
	vec2(-1.0f, -1.0f),
	vec2(-1.0f,  1.0f),
	vec2( 1.0f, -1.0f),
	vec2( 1.0f,  1.0f),
};

void vertexMain()
{
	float lifetime = pow(clamp(in_position.w, 0.0f, 1.0f), 0.1f);
	float size = PARTICLE_SIZE * lifetime;

	vec2 offset = vertices[gl_VertexIndex];

	vec4 view_position = camera.view * vec4(in_position.xyz, 1.0);
	view_position.x += offset.x * size;
	view_position.y += offset.y * size;

	gl_Position = camera.projection * view_position;

	out_data = vec4(offset.x, offset.y, in_position.w, 0.0f);
}
