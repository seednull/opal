#version 450

layout(location = 0) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform Application
{
	vec4 viewport;
	float time;
} application;

layout(set = 0, binding = 1) uniform sampler2D hdriSampler;

const int MAX_STEPS = 100;
const int MAX_TRANSMITTANCE_STEPS = 8;
const float START_DEPTH = 0.01f;
const float END_DEPTH = 10.0f;

const float PI =  3.141592653589798979f;
const float PI2 = 6.283185307179586477f;
const float iPI = 0.318309886183790672f;
const float iPI2 = 0.1591549430918953357f;
const float EPSILON = 1e-3f;
const vec2 ATAN_INV = vec2(iPI2, iPI);

const float GAMMA = 2.2f;
const float INV_GAMMA = (1.0f / GAMMA);

float saturate(float x)
{
	return clamp(x, 0.0f, 1.0f);
}

vec3 saturate(vec3 x)
{
	return clamp(x, 0.0f, 1.0f);
}

vec3 gamma(vec3 col)
{
	return pow(col, vec3(INV_GAMMA));
}

vec3 inv_gamma(vec3 col)
{
	return pow(col, vec3(GAMMA));
}

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

vec2 equirectangularUV(vec3 v)
{
	vec2 uv = vec2(atan(v.x, v.y), -asin(v.z));
	uv *= ATAN_INV;
	uv += 0.5;
	return uv;
}

vec3 bump3y (vec3 x, vec3 yoffset)
{
	vec3 y = vec3(1) - x * x;
	y = saturate(y - yoffset);
	return y;
}

vec3 getRainbowGradient(float w)
{
	if(w > 700.0 || w < 400.0)
	{
		return vec3(0);
	}

	float x = saturate((w - 400.0)/ 300.0);

	const vec3 c1 = vec3(3.54585104, 2.93225262, 2.41593945);
	const vec3 x1 = vec3(0.69549072, 0.49228336, 0.27699880);
	const vec3 y1 = vec3(0.02312639, 0.15225084, 0.52607955);

	const vec3 c2 = vec3(3.90307140, 3.21182957, 3.96587128);
	const vec3 x2 = vec3(0.11748627, 0.86755042, 0.66077860);
	const vec3 y2 = vec3(0.84897130, 0.88445281, 0.73949448);

	vec3 col = bump3y(c1 * (x - x1), y1) + bump3y(c2 * (x - x2), y2);

	col = inv_gamma(col);
	return col;
}

vec3 opalIridescence(vec3 view, vec3 normal, vec3 light, vec3 grating, float d)
{
	vec3 color = vec3(0);
	
	if (dot(normal, light) < 0.0 || dot(normal, view) < 0.0)
	{
		return color;
	}

	float sin_theta_l = dot(grating, light);
	float sin_theta_v = dot(grating, view);

	float distance_difference = abs(d * sin_theta_l - d * sin_theta_v);

	float wavelengths[9] = float[9](400.0, 437.0, 474.0, 511.0, 548.0, 585.0, 622.0, 659.0, 700.0);
	float intensities[9] = float[9](1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0);

	for (int i = 0; i < 9; i++)
	{
		float wavelength = wavelengths[i];
		float intensity = intensities[i];
		vec3 wavelength_color = getRainbowGradient(wavelength) * intensity;

		for (int n = 1; n <= 2; n++)
		{
			float target_distance = wavelength * float(n);
			float interference = 1.0f - saturate(abs(target_distance - distance_difference) / (37.0 * float(n)));
			color += wavelength_color * interference;
		}
	}

	return saturate(color);
}

vec3 opalTransmittance(vec3 color, vec3 p, vec3 direction, float max_distance)
{
	float transmittance = 1.0f;
	float step_size = max_distance / MAX_TRANSMITTANCE_STEPS;

	for (int i = 0; i < MAX_TRANSMITTANCE_STEPS; ++i)
	{
		vec3 origin = p + direction * step_size * i;
		transmittance *= exp(-step_size * 5.0f);
	}

	return color * transmittance;
}

float world(vec3 p, float sdf_sign)
{
	vec3 scale = vec3(1.0f, 0.8f, 0.4f);
	float min_scale = min(scale.x, min(scale.y, scale.z));

	float sdf_distance = opSmoothDifference(
		sdSphere(p / scale, vec3(0.0f, 0.0f, 0.0f), 1.0f) * min_scale,
		sdPlane(p, vec4(0.0f, 0.0f, 1.0f, 0.0f)),
		0.01f
	);

	return sdf_distance * sdf_sign;
}

vec3 normal(vec3 p, float sdf_sign)
{
	vec3 n;
	n.x = world(vec3(p.x + EPSILON, p.y, p.z), sdf_sign) - world(vec3(p.x - EPSILON, p.y, p.z), sdf_sign);
	n.y = world(vec3(p.x, p.y + EPSILON, p.z), sdf_sign) - world(vec3(p.x, p.y - EPSILON, p.z), sdf_sign);
	n.z = world(vec3(p.x, p.y, p.z + EPSILON), sdf_sign) - world(vec3(p.x, p.y, p.z - EPSILON), sdf_sign);

	return normalize(n);
}

float trace(vec3 origin, vec3 direction, float start, float end, float sdf_sign)
{
	float depth = start;
	for (int i = 0; i < MAX_STEPS; i++)
	{
		float dist = world(origin + direction * depth, sdf_sign);

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

	float phi = mod(application.time * 0.2f, PI2);
	float theta = PI * 0.15f;
	float radius = 3.0f;

	vec3 ray_target = vec3(0.0f, 0.0f, 0.0f);
	vec3 ray_origin = vec3(0.0f, 0.0f, 0.0f);

	ray_origin.x += cos(phi) * cos(theta);
	ray_origin.y += sin(phi) * cos(theta);
	ray_origin.z += sin(theta);
	ray_origin *= radius;

	vec3 up = vec3(0.0f, 0.0f, 1.0f);
	vec3 forward = normalize(ray_target - ray_origin);
	vec3 right = normalize(cross(forward, up));

	vec3 ray_direction = normalize(forward + up * offset.y + right * offset.x);

	vec4 color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec3 light_direction = normalize(vec3(0.0f, -1.0f, -1.0f));

	float distance_near = trace(ray_origin, ray_direction, START_DEPTH, END_DEPTH, 1.0f);
	if (distance_near != END_DEPTH)
	{
		vec3 p_near = ray_origin + ray_direction * distance_near;
		float distance_far = trace(p_near, ray_direction, START_DEPTH, END_DEPTH, -1.0f);

		vec3 p_far = p_near + ray_direction * distance_far;

		vec3 n_near = normal(p_near, 1.0f);
		vec3 n_far = normal(p_far, -1.0f);
		vec3 grating = n_near;

		vec3 refracted = normalize(refract(ray_direction, n_far, 0.5f));
		vec3 transmitted = texture(hdriSampler, equirectangularUV(refracted)).rgb;

		vec3 transmittance = opalTransmittance(transmitted, p_near, ray_direction, distance_far);
		vec3 iridescence = opalIridescence(-ray_direction, n_near, -light_direction, grating, 700.0f);

		color.rgb = transmittance + iridescence;
	}
	else
	{
		color = texture(hdriSampler, equirectangularUV(ray_direction));
	}

	color.rgb = gamma(color.rgb);
	out_color = color;
}
