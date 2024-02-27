#pragma once

#include <stdint.h> // TODO: get rid of this dependency later

// Version
#define OPAL_VERSION_MAJOR 1
#define OPAL_VERSION_MINOR 0
#define OPAL_VERSION_PATCH 0
#define OPAL_VERSION "1.0.0-dev"

// Platform specific defines
#if defined(_WIN32)
	#define OPAL_EXPORT		__declspec(dllexport)
	#define OPAL_IMPORT		__declspec(dllimport)
	#define OPAL_INLINE		__forceinline
	#define OPAL_RESTRICT	__restrict
#else
	#define OPAL_EXPORT		__attribute__((visibility("default")))
	#define OPAL_IMPORT
	#define OPAL_INLINE		__inline__
	#define OPAL_RESTRICT	__restrict
#endif

#if defined(OPAL_SHARED_LIBRARY)
	#define OPAL_APIENTRY OPAL_EXPORT extern
#else
	#define OPAL_APIENTRY OPAL_IMPORT extern
#endif

#if !defined(OPAL_NULL_HANDLE)
	#define OPAL_NULL_HANDLE 0
#endif

#define OPAL_DEFINE_HANDLE(TYPE) typedef uint64_t TYPE;

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handles
OPAL_DEFINE_HANDLE(Opal_Instance);
OPAL_DEFINE_HANDLE(Opal_Device);

// Enums
typedef enum Opal_Result_t
{
	OPAL_SUCCESS = 0,
	OPAL_NOT_SUPPORTED,
	OPAL_INVALID_INSTANCE,
	OPAL_INVALID_DEVICE,
	OPAL_INVALID_DEVICE_INDEX,

	// FIXME: add more error codes for dxgi / d3d12 stuff
	OPAL_DIRECX12_ERROR,

	// FIXME: add more error codes for webgpu stuff
	OPAL_WEBGPU_ERROR,

	// FIXME: add more error codes for vulkan stuff
	OPAL_VULKAN_ERROR,

	OPAL_RESULT_MAX,
} Opal_Result;

typedef enum Opal_Api_t
{
	OPAL_API_VULKAN = 0,
	OPAL_API_DIRECTX12,
	OPAL_API_METAL,
	OPAL_API_WEBGPU,
	OPAL_API_NULL,

	OPAL_API_MAX,
	OPAL_API_DEFAULT = OPAL_API_VULKAN,
} Opal_Api;

typedef enum Opal_DeviceHint_t
{
	OPAL_DEVICE_HINT_DEFAULT = 0,
	OPAL_DEVICE_HINT_PREFER_HIGH_PERFORMANCE,
	OPAL_DEVICE_HINT_PREFER_LOW_POWER,

	OPAL_DEFAULT_DEVICE_MAX,
} Opal_DeviceHint;

typedef enum Opal_GpuType_t
{
	OPAL_GPU_TYPE_DISCRETE = 0,
	OPAL_GPU_TYPE_INTEGRATED,
	OPAL_GPU_TYPE_CPU,
	OPAL_GPU_TYPE_EXTERNAL,
	OPAL_GPU_TYPE_UNKNOWN,

	OPAL_GPU_TYPE_MAX,
} Opal_GpuType;

// Structs
typedef struct Opal_DeviceInfo_t
{
	char name[256];
	Opal_GpuType gpu_type;
	uint64_t driver_version;
	uint32_t vendor_id;
	uint32_t device_id;
	uint8_t tessellation_shader : 1;
	uint8_t geometry_shader : 1;
	uint8_t compute_shader : 1;
	uint8_t mesh_pipeline : 1;
	uint8_t raytrace_pipeline : 1;
	uint8_t texture_compression_etc2 : 1;
	uint8_t texture_compression_astc : 1;
	uint8_t texture_compression_bc : 1;
	uint64_t max_buffer_alignment;
} Opal_DeviceInfo;

typedef struct Opal_InstanceDesc_t
{
	const char *application_name;
	uint32_t application_version;
	const char *engine_name;
	uint32_t engine_version;
} Opal_InstanceDesc;

// Function pointers
typedef Opal_Result (*PFN_opalCreateInstance)(Opal_Api api, const Opal_InstanceDesc *desc, Opal_Instance *instance);
typedef Opal_Result (*PFN_opalDestroyInstance)(Opal_Instance instance);

typedef Opal_Result (*PFN_opalEnumerateDevices)(Opal_Instance instance, uint32_t *device_count, Opal_DeviceInfo *infos);

typedef Opal_Result (*PFN_opalCreateDevice)(Opal_Instance instance, uint32_t index, Opal_Device *device);
typedef Opal_Result (*PFN_opalCreateDefaultDevice)(Opal_Instance instance, Opal_DeviceHint hint, Opal_Device *device);
typedef Opal_Result (*PFN_opalDestroyDevice)(Opal_Device device);
typedef Opal_Result (*PFN_opalGetDeviceInfo)(Opal_Device device, Opal_DeviceInfo *info);

// API
#if !defined(OPAL_NO_PROTOTYPES)
OPAL_APIENTRY Opal_Result opalCreateInstance(Opal_Api api, const Opal_InstanceDesc *desc, Opal_Instance *instance);
OPAL_APIENTRY Opal_Result opalDestroyInstance(Opal_Instance instance);

OPAL_APIENTRY Opal_Result opalEnumerateDevices(Opal_Instance instance, uint32_t *device_count, Opal_DeviceInfo *infos);

OPAL_APIENTRY Opal_Result opalCreateDevice(Opal_Instance instance, uint32_t index, Opal_Device *device);
OPAL_APIENTRY Opal_Result opalCreateDefaultDevice(Opal_Instance instance, Opal_DeviceHint hint, Opal_Device *device);
OPAL_APIENTRY Opal_Result opalDestroyDevice(Opal_Device device);
OPAL_APIENTRY Opal_Result opalGetDeviceInfo(Opal_Device device, Opal_DeviceInfo *info);
#endif

#ifdef __cplusplus
}
#endif
