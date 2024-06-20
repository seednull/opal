#version 450

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform Application
{
	vec4 viewport;
	float time;
} application;

const int MAX_STEPS = 100;
const float EPSILON = 0.001f;
const float START_DEPTH = 0.0f;
const float END_DEPTH = 10.0f;

float sdSphere(vec3 p, vec3 center, float radius)
{
	return distance(p, center) - radius;
}

float sdPlane(vec3 p, vec4 plane)
{
	return dot(vec4(p, 1.0f), plane);
}

float opUnion(float d1, float d2)
{
	return min(d1, d2);
}

float opIntersection(float d1, float d2)
{
	return max(d1, d2);
}

float opDifference(float d1, float d2)
{
	return max(d1, -d2);
}

float opSmoothDifference(float d1, float d2, float smoothness)
{
	float k = smoothness * 4.0f;
	float h = max(k - abs(d1 + d2), 0.0f) / k;

	return opDifference(d1, d2) + h * h * k * 0.25f;
}

float world(vec3 p)
{
	vec3 scale = vec3(1.0f, 1.0f, 0.5f);
	float min_scale = min(scale.x, min(scale.y, scale.z));

	return opSmoothDifference(
		sdSphere(p / scale, vec3(2.0f, 0.0f, 0.0f), 1.0f) * min_scale,
		sdPlane(p, vec4(0.0f, 0.0f, 1.0f, 0.0f)),
		0.01f
	);
}

vec3 normal(vec3 p)
{
	vec3 n;
	n.x = world(vec3(p.x + EPSILON, p.y, p.z)) - world(vec3(p.x - EPSILON, p.y, p.z));
	n.y = world(vec3(p.x, p.y + EPSILON, p.z)) - world(vec3(p.x, p.y - EPSILON, p.z));
	n.z = world(vec3(p.x, p.y, p.z + EPSILON)) - world(vec3(p.x, p.y, p.z - EPSILON));

	return normalize(n);
}

float trace(vec3 origin, vec3 direction, float start, float end)
{
	float depth = start;
	for (int i = 0; i < MAX_STEPS; i++)
	{
		float dist = world(origin + direction * depth);

		if (dist < EPSILON)
			return depth;

		depth += dist;

		if (depth >= END_DEPTH)
			return end;
	}

	return end;
}

void main()
{
	vec2 offset = in_uv * 2.0f - vec2(1.0f, 1.0f);
	offset.x *= application.viewport.x * application.viewport.w;

	vec3 ray_origin = vec3(0.0f, 0.0f, 0.4f);
	vec3 ray_direction = normalize(vec3(1.0f, offset.x, offset.y));

	vec4 color = vec4(0.0f, 0.0f, 0.0f, 1.0f);

	float depth = trace(ray_origin, ray_direction, START_DEPTH, END_DEPTH);
	if (depth != END_DEPTH)
	{
		vec3 n = normal(ray_origin + ray_direction * depth);
		n = n * 0.5f + vec3(0.5f);

		color = vec4(n, 1.0f);
	}

	color.rgb = pow(color.rgb, vec3(1.0f / 2.2f));
	out_color = color;
}
