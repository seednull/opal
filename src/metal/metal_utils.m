#include "metal_internal.h"

static const uint32_t apple_vendor_id = 0x106b;
static const uint32_t display_vga_class = 0x30000;

/*
 */
static uint32_t metal_getEntryProperty(io_registry_entry_t entry, CFStringRef name)
{
	CFTypeRef property = IORegistryEntrySearchCFProperty(entry, kIOServicePlane, name, kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents);
	if (!property)
		return 0;

	uint32_t value = 0;
	uint32_t *ptr = (uint32_t *)CFDataGetBytePtr((CFDataRef)property);
	if (ptr)
		value = *ptr;

	CFRelease(property);

	return value;
}

static bool metal_tryFillByRegistryID(uint64_t registry_id, Opal_DeviceInfo *info)
{
	io_registry_entry_t entry = IOServiceGetMatchingService(MACH_PORT_NULL, IORegistryEntryIDMatching(registry_id));
	if (!entry)
		return false;

	io_registry_entry_t parent;
	if (IORegistryEntryGetParentEntry(entry, kIOServicePlane, &parent) != kIOReturnSuccess)
	{
		IOObjectRelease(entry);
		return false;
	}

	info->vendor_id = metal_getEntryProperty(entry, CFSTR("vendor-id"));
	info->device_id = metal_getEntryProperty(entry, CFSTR("device-id"));

	IOObjectRelease(parent);
	IOObjectRelease(entry);

	return true;
}

static bool metal_tryFillBySearchingAll(Opal_DeviceInfo *info)
{
	io_iterator_t entry_iterator;
	if (IOServiceGetMatchingServices(MACH_PORT_NULL, IOServiceMatching("IOCIDevice"), &entry_iterator) != kIOReturnSuccess)
		return false;

	while (true)
	{
		io_registry_entry_t entry = IOIteratorNext(entry_iterator);
		if (!entry)
			break;

		uint32_t class = metal_getEntryProperty(entry, CFSTR("class-code"));
		if (class != display_vga_class)
			continue;

		info->vendor_id = metal_getEntryProperty(entry, CFSTR("vendor-id"));
		info->device_id = metal_getEntryProperty(entry, CFSTR("device-id"));
		break;
	}

	IOObjectRelease(entry_iterator);
	return false;
}

uint32_t metal_getAppleDeviceID(MTLGPUFamily family)
{
	if (family == 0)
		return 0;

	NSOperatingSystemVersion os = [[NSProcessInfo processInfo] operatingSystemVersion];
	uint8_t os_major = (uint8_t)os.majorVersion;
	uint8_t os_minor = (uint8_t)os.minorVersion;

	return (os_major << 24) + (os_minor << 16) + (uint16_t)family;
}

MTLGPUFamily metal_getAppleDeviceFamily(id<MTLDevice> metal_device)
{
	assert(metal_device);

	static MTLGPUFamily supported_families[] =
	{
		MTLGPUFamilyMac2,
		MTLGPUFamilyApple1,
		MTLGPUFamilyApple2,
		MTLGPUFamilyApple3,
		MTLGPUFamilyApple4,
		MTLGPUFamilyApple5,
		MTLGPUFamilyApple6,
		MTLGPUFamilyApple7,
		MTLGPUFamilyApple8,
		MTLGPUFamilyApple9,
	};

	static uint32_t num_families = sizeof(supported_families) / sizeof(MTLGPUFamily);

	MTLGPUFamily family = 0;

	for (uint32_t i = 0; i < num_families; ++i)
		if ([metal_device supportsFamily: supported_families[i]])
			family = supported_families[i];

	return family;
}

/*
 */
Opal_Result metal_helperFillDeviceInfo(id<MTLDevice> metal_device, Opal_DeviceInfo *info)
{
	assert(metal_device);
	assert(info);

	memset(info, 0, sizeof(Opal_DeviceInfo));

	bool is_common2_or_greater = [metal_device supportsFamily: MTLGPUFamilyCommon2];
	bool is_metal3 = [metal_device supportsFamily: MTLGPUFamilyMetal3];

	MTLGPUFamily family = metal_getAppleDeviceFamily(metal_device);

	bool is_apple1_or_greater = family >= MTLGPUFamilyApple1;
	bool is_apple2_or_greater = family >= MTLGPUFamilyApple2;
	bool is_apple3_or_greater = family >= MTLGPUFamilyApple3;
	bool is_apple7_or_greater = family >= MTLGPUFamilyApple7;
	bool is_mac2 = family == MTLGPUFamilyMac2;

	strncpy(info->name, metal_device.name.UTF8String, 256);

	info->api = OPAL_API_METAL;
	info->driver_version = 0; // FIXME: fill with hardcoded Opal version

	if (family)
	{
		info->vendor_id = apple_vendor_id;
		info->device_id = metal_getAppleDeviceID(family);
	}
	else
	{
		if (!metal_tryFillByRegistryID(metal_device.registryID, info))
			metal_tryFillBySearchingAll(info);
	}

	info->device_type = OPAL_DEVICE_TYPE_DISCRETE;

	if (metal_device.isLowPower)
		info->device_type = OPAL_DEVICE_TYPE_INTEGRATED;

	info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_MAIN] = 16; // NOTE: intentional artificial limit in order to keep the API consistent
	info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_COMPUTE] = 8; // NOTE: intentional artificial limit in order to keep the API consistent
	info->features.queue_count[OPAL_DEVICE_ENGINE_TYPE_COPY] = 2; // NOTE: intentional artificial limit in order to keep the API consistent
	info->features.tessellation_shader = is_apple3_or_greater || is_mac2 || is_common2_or_greater;
	info->features.compute_pipeline = 1;
	info->features.meshlet_pipeline = is_apple7_or_greater || is_mac2 || is_metal3;
	info->features.raytrace_pipeline = metal_device.supportsRaytracing;
	info->features.texture_compression_etc2 = is_apple2_or_greater && !is_mac2;
	info->features.texture_compression_astc = is_apple2_or_greater && !is_mac2;
	info->features.texture_compression_bc = metal_device.supportsBCTextureCompression;

	// TODO: fill limits properly
	info->limits.max_texture_dimension_1d = 0;
	info->limits.max_texture_dimension_1d = 0;
	info->limits.max_texture_dimension_2d = 0;
	info->limits.max_texture_dimension_3d = 0;
	info->limits.max_texture_array_layers = 0;
	info->limits.max_buffer_size = 0;
	info->limits.min_uniform_buffer_offset_alignment = 0;
	info->limits.min_storage_buffer_offset_alignment = 0;
	info->limits.max_descriptor_sets = 0;
	info->limits.max_uniform_buffer_binding_size = 0;
	info->limits.max_storage_buffer_binding_size = 0;
	info->limits.max_vertex_buffers = 0;
	info->limits.max_vertex_attributes = 0;
	info->limits.max_vertex_buffer_stride = 0;
	info->limits.max_color_attachments = 0;
	info->limits.max_compute_shared_memory_size = 0;
	info->limits.max_compute_workgroup_count_x = 0;
	info->limits.max_compute_workgroup_count_y = 0;
	info->limits.max_compute_workgroup_count_z = 0;
	info->limits.max_compute_workgroup_invocations = 0;
	info->limits.max_compute_workgroup_local_size_x = 0;
	info->limits.max_compute_workgroup_local_size_y = 0;
	info->limits.max_compute_workgroup_local_size_z = 0;
	info->limits.max_raytrace_recursion_depth = 0;
	info->limits.max_raytrace_hit_attribute_size = 0;

	return OPAL_SUCCESS;
}
