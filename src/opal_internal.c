#include "opal_internal.h"

#include <assert.h>

static uint32_t device_type_default_scores[] =
{
	1000,
	100,
	0,
	0,
	0,
};

static uint32_t device_type_high_performance_scores[] =
{
	1000,
	100,
	0,
	0,
	0,
};

static uint32_t device_type_low_power_scores[] =
{
	100,
	1000,
	0,
	0,
	0,
};

/*
 */
uint32_t opal_evaluateDevice(const Opal_DeviceInfo *info, Opal_DeviceHint hint)
{
	assert(info);
	
	uint32_t score = 0;

	switch (hint)
	{
		case OPAL_DEVICE_HINT_PREFER_HIGH_PERFORMANCE: score += device_type_high_performance_scores[info->device_type]; break;
		case OPAL_DEVICE_HINT_PREFER_LOW_POWER: score += device_type_low_power_scores[info->device_type]; break;
		default: score += device_type_default_scores[info->device_type]; break;
	}

	score += info->tessellation_shader;
	score += info->geometry_shader;
	score += info->compute_pipeline * 10;
	score += info->meshlet_pipeline * 10;
	score += info->raytrace_pipeline * 10;

	return score;
}

#if !defined(OPAL_BACKEND_DIRECTX12)

Opal_Result directx12_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	OPAL_UNUSED(desc);
	OPAL_UNUSED(instance);

	return OPAL_NOT_SUPPORTED;
}

#endif

#if !defined(OPAL_BACKEND_WEBGPU)

Opal_Result webgpu_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	OPAL_UNUSED(desc);
	OPAL_UNUSED(instance);

	return OPAL_NOT_SUPPORTED;
}

#endif

#if !defined(OPAL_BACKEND_VULKAN)

Opal_Result vulkan_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	OPAL_UNUSED(desc);
	OPAL_UNUSED(instance);

	return OPAL_NOT_SUPPORTED;
}

#endif

#if !defined(OPAL_BACKEND_METAL)

Opal_Result metal_createInstance(const Opal_InstanceDesc *desc, Opal_Instance *instance)
{
	assert(desc);
	assert(instance);

	OPAL_UNUSED(desc);
	OPAL_UNUSED(instance);

	return OPAL_NOT_SUPPORTED;
}

#endif
