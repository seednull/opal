#include "opal_internal.h"

static uint32_t gpu_type_default_scores[] = {
	1000,
	100,
	0,
	0,
	0,
};

static uint32_t gpu_type_high_performance_scores[] = {
	1000,
	100,
	0,
	0,
	0,
};

static uint32_t gpu_type_low_power_scores[] = {
	100,
	1000,
	0,
	0,
	0,
};

/*
 */
uint32_t opalEvaluateDevice(const Opal_DeviceInfo *info, Opal_DeviceHint hint)
{
	assert(info);
	
	uint32_t score = 0;

	switch (hint)
	{
		case OPAL_DEVICE_HINT_PREFER_HIGH_PERFORMANCE: score += gpu_type_high_performance_scores[info->gpu_type]; break;
		case OPAL_DEVICE_HINT_PREFER_LOW_POWER: score += gpu_type_low_power_scores[info->gpu_type]; break;
		default: score += gpu_type_default_scores[info->gpu_type]; break;
	}

	score += info->tessellation_shader;
	score += info->geometry_shader;
	score += info->compute_shader * 10;
	score += info->mesh_pipeline * 10;
	score += info->raytrace_pipeline * 10;

	return score;
}
