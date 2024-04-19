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

// Constants
#define OPAL_DEFAULT_HEAP_SIZE 0x10000000
#define OPAL_DEFAULT_HEAP_ALLOCATIONS 1000000
#define OPAL_DEFAULT_HEAPS 64

// Opaque handles
OPAL_DEFINE_HANDLE(Opal_Instance);
OPAL_DEFINE_HANDLE(Opal_Device);
OPAL_DEFINE_HANDLE(Opal_Buffer);
OPAL_DEFINE_HANDLE(Opal_Texture);
OPAL_DEFINE_HANDLE(Opal_TextureView);

// Enums
typedef enum Opal_Result_t
{
	OPAL_SUCCESS = 0,
	OPAL_NOT_SUPPORTED,
	OPAL_INVALID_INSTANCE,
	OPAL_INVALID_DEVICE,
	OPAL_INVALID_DEVICE_INDEX,
	OPAL_INVALID_BUFFER,
	OPAL_NO_MEMORY,

	// FIXME: add more error codes for internal errors
	OPAL_INTERNAL_ERROR,

	// FIXME: add more error codes for dxgi / d3d12 stuff
	OPAL_DIRECX12_ERROR,

	// FIXME: add more error codes for webgpu stuff
	OPAL_WEBGPU_ERROR,

	// FIXME: add more error codes for vulkan stuff
	OPAL_VULKAN_ERROR,

	// FIXME: add more error codes for metal stuff
	OPAL_METAL_ERROR,

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

typedef enum Opal_AllocationMemoryType_t
{
	OPAL_ALLOCATION_MEMORY_TYPE_PRIVATE = 0,
	OPAL_ALLOCATION_MEMORY_TYPE_STREAM,
	OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
	OPAL_ALLOCATION_MEMORY_TYPE_READBACK,

	OPAL_ALLOCATION_MEMORY_TYPE_MAX,
} Opal_AllocationMemoryType;

typedef enum Opal_AllocationHint_t
{
	OPAL_ALLOCATION_HINT_AUTO,
	OPAL_ALLOCATION_HINT_PREFER_DEDICATED,
	OPAL_ALLOCATION_HINT_PREFER_HEAP,

	OPAL_ALLOCATION_HINT_MAX,
} Opal_AllocationHint;

typedef enum Opal_BufferUsageFlags_t
{
	OPAL_BUFFER_USAGE_TRANSFER_SRC = 0x0001,
	OPAL_BUFFER_USAGE_TRANSFER_DST = 0x0002,
	OPAL_BUFFER_USAGE_VERTEX = 0x0004,
	OPAL_BUFFER_USAGE_INDEX = 0x0008,
	OPAL_BUFFER_USAGE_UNIFORM = 0x0010,
	OPAL_BUFFER_USAGE_STORAGE = 0x0020,
	OPAL_BUFFER_USAGE_INDIRECT = 0x0040,
} Opal_BufferUsageFlags;

typedef enum Opal_TextureUsageFlags_t
{
	OPAL_TEXTURE_USAGE_TRANSFER_SRC = 0x0001,
	OPAL_TEXTURE_USAGE_TRANSFER_DST = 0x0002,
	OPAL_TEXTURE_USAGE_SHADER_SAMPLED = 0x0004,
	OPAL_TEXTURE_USAGE_SHADER_STORAGE = 0x0008,
	OPAL_TEXTURE_USAGE_RENDER_ATTACHMENT = 0x0010,
} Opal_TextureUsageFlags;

typedef enum Opal_TextureType_t
{
	OPAL_TEXTURE_TYPE_1D = 0,
	OPAL_TEXTURE_TYPE_2D,
	OPAL_TEXTURE_TYPE_2D_ARRAY,
	OPAL_TEXTURE_TYPE_3D,

	OPAL_TEXTURE_TYPE_MAX,
} Opal_TextureType;

typedef enum Opal_TextureViewType_t
{
	OPAL_TEXTURE_VIEW_TYPE_1D = 0,
	OPAL_TEXTURE_VIEW_TYPE_2D,
	OPAL_TEXTURE_VIEW_TYPE_2D_ARRAY,
	OPAL_TEXTURE_VIEW_TYPE_CUBE,
	OPAL_TEXTURE_VIEW_TYPE_CUBE_ARRAY,
	OPAL_TEXTURE_VIEW_TYPE_3D,

	OPAL_TEXTURE_VIEW_TYPE_MAX,
} Opal_TextureViewType;

typedef enum Opal_TextureSamples_t
{
	OPAL_TEXTURE_SAMPLES_1 = 0,
	OPAL_TEXTURE_SAMPLES_2,
	OPAL_TEXTURE_SAMPLES_4,
	OPAL_TEXTURE_SAMPLES_8,
	OPAL_TEXTURE_SAMPLES_16,
	OPAL_TEXTURE_SAMPLES_32,
	OPAL_TEXTURE_SAMPLES_64,

	OPAL_TEXTURE_SAMPLES_MAX,
} Opal_TextureSamples;

typedef enum Opal_TextureFormat_t
{
	OPAL_TEXTURE_FORMAT_UNDEFINED = 0,

	OPAL_TEXTURE_FORMAT_R8_UNORM,
	OPAL_TEXTURE_FORMAT_R8_SNORM,
	OPAL_TEXTURE_FORMAT_R8_UINT,
	OPAL_TEXTURE_FORMAT_R8_SINT,
	OPAL_TEXTURE_FORMAT_R8_SRGB,

	OPAL_TEXTURE_FORMAT_RG8_UNORM,
	OPAL_TEXTURE_FORMAT_RG8_SNORM,
	OPAL_TEXTURE_FORMAT_RG8_UINT,
	OPAL_TEXTURE_FORMAT_RG8_SINT,
	OPAL_TEXTURE_FORMAT_RG8_SRGB,

	OPAL_TEXTURE_FORMAT_RGB8_UNORM,
	OPAL_TEXTURE_FORMAT_RGB8_SNORM,
	OPAL_TEXTURE_FORMAT_RGB8_UINT,
	OPAL_TEXTURE_FORMAT_RGB8_SINT,
	OPAL_TEXTURE_FORMAT_RGB8_SRGB,

	OPAL_TEXTURE_FORMAT_RGBA8_UNORM,
	OPAL_TEXTURE_FORMAT_RGBA8_SNORM,
	OPAL_TEXTURE_FORMAT_RGBA8_UINT,
	OPAL_TEXTURE_FORMAT_RGBA8_SINT,
	OPAL_TEXTURE_FORMAT_RGBA8_SRGB,

	OPAL_TEXTURE_FORMAT_BGRA8_UNORM,
	OPAL_TEXTURE_FORMAT_BGRA8_SNORM,
	OPAL_TEXTURE_FORMAT_BGRA8_UINT,
	OPAL_TEXTURE_FORMAT_BGRA8_SINT,
	OPAL_TEXTURE_FORMAT_BGRA8_SRGB,

	OPAL_TEXTURE_FORMAT_R16_UNORM,
	OPAL_TEXTURE_FORMAT_R16_SNORM,
	OPAL_TEXTURE_FORMAT_R16_UINT,
	OPAL_TEXTURE_FORMAT_R16_SINT,
	OPAL_TEXTURE_FORMAT_R16_SFLOAT,

	OPAL_TEXTURE_FORMAT_RG16_UNORM,
	OPAL_TEXTURE_FORMAT_RG16_SNORM,
	OPAL_TEXTURE_FORMAT_RG16_UINT,
	OPAL_TEXTURE_FORMAT_RG16_SINT,
	OPAL_TEXTURE_FORMAT_RG16_SFLOAT,

	OPAL_TEXTURE_FORMAT_RGB16_UNORM,
	OPAL_TEXTURE_FORMAT_RGB16_SNORM,
	OPAL_TEXTURE_FORMAT_RGB16_UINT,
	OPAL_TEXTURE_FORMAT_RGB16_SINT,
	OPAL_TEXTURE_FORMAT_RGB16_SFLOAT,

	OPAL_TEXTURE_FORMAT_RGBA16_UNORM,
	OPAL_TEXTURE_FORMAT_RGBA16_SNORM,
	OPAL_TEXTURE_FORMAT_RGBA16_UINT,
	OPAL_TEXTURE_FORMAT_RGBA16_SINT,
	OPAL_TEXTURE_FORMAT_RGBA16_SFLOAT,

	OPAL_TEXTURE_FORMAT_R32_UINT,
	OPAL_TEXTURE_FORMAT_R32_SINT,
	OPAL_TEXTURE_FORMAT_R32_SFLOAT,

	OPAL_TEXTURE_FORMAT_RG32_UINT,
	OPAL_TEXTURE_FORMAT_RG32_SINT,
	OPAL_TEXTURE_FORMAT_RG32_SFLOAT,

	OPAL_TEXTURE_FORMAT_RGB32_UINT,
	OPAL_TEXTURE_FORMAT_RGB32_SINT,
	OPAL_TEXTURE_FORMAT_RGB32_SFLOAT,

	OPAL_TEXTURE_FORMAT_RGBA32_UINT,
	OPAL_TEXTURE_FORMAT_RGBA32_SINT,
	OPAL_TEXTURE_FORMAT_RGBA32_SFLOAT,

	OPAL_TEXTURE_FORMAT_B10G11R11_UFLOAT,
	OPAL_TEXTURE_FORMAT_E5B9G9R9_UFLOAT,

	OPAL_TEXTURE_FORMAT_D16_UNORM,
	OPAL_TEXTURE_FORMAT_D32_SFLOAT,
	OPAL_TEXTURE_FORMAT_D16_UNORM_S8_UINT,
	OPAL_TEXTURE_FORMAT_D24_UNORM_S8_UINT,
	OPAL_TEXTURE_FORMAT_D32_SFLOAT_S8_UINT,

	OPAL_TEXTURE_FORMAT_BC1_RGB_UNORM,
	OPAL_TEXTURE_FORMAT_BC1_RGB_SRGB,
	OPAL_TEXTURE_FORMAT_BC1_RGBA_UNORM,
	OPAL_TEXTURE_FORMAT_BC1_RGBA_SRGB,
	OPAL_TEXTURE_FORMAT_BC2_UNORM,
	OPAL_TEXTURE_FORMAT_BC2_SRGB,
	OPAL_TEXTURE_FORMAT_BC3_UNORM,
	OPAL_TEXTURE_FORMAT_BC3_SRGB,
	OPAL_TEXTURE_FORMAT_BC4_UNORM,
	OPAL_TEXTURE_FORMAT_BC4_SNORM,
	OPAL_TEXTURE_FORMAT_BC5_UNORM,
	OPAL_TEXTURE_FORMAT_BC5_SNORM,
	OPAL_TEXTURE_FORMAT_BC6H_UFLOAT,
	OPAL_TEXTURE_FORMAT_BC6H_SFLOAT,
	OPAL_TEXTURE_FORMAT_BC7_UNORM,
	OPAL_TEXTURE_FORMAT_BC7_SRGB,

	OPAL_TEXTURE_FORMAT_ETC2_R8G8B8_UNORM,
	OPAL_TEXTURE_FORMAT_ETC2_R8G8B8_SRGB,
	OPAL_TEXTURE_FORMAT_ETC2_R8G8B8A1_UNORM,
	OPAL_TEXTURE_FORMAT_ETC2_R8G8B8A1_SRGB,
	OPAL_TEXTURE_FORMAT_ETC2_R8G8B8A8_UNORM,
	OPAL_TEXTURE_FORMAT_ETC2_R8G8B8A8_SRGB,
	OPAL_TEXTURE_FORMAT_EAC_R11_UNORM,
	OPAL_TEXTURE_FORMAT_EAC_R11_SNORM,
	OPAL_TEXTURE_FORMAT_EAC_R11G11_UNORM,
	OPAL_TEXTURE_FORMAT_EAC_R11G11_SNORM,

	OPAL_TEXTURE_FORMAT_ASTC_4x4_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_4x4_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_5x4_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_5x4_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_5x5_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_5x5_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_6x5_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_6x5_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_6x6_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_6x6_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_8x5_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_8x5_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_8x6_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_8x6_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_8x8_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_8x8_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_10x5_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_10x5_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_10x6_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_10x6_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_10x8_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_10x8_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_10x10_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_10x10_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_12x10_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_12x10_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_12x12_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_12x12_SRGB,

	OPAL_TEXTURE_FORMAT_MAX,
} Opal_TextureFormat;

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
	const char *engine_name;
	uint32_t application_version;
	uint32_t engine_version;
	uint32_t heap_size;
	uint32_t max_heap_allocations;
	uint32_t max_heaps;
} Opal_InstanceDesc;

typedef struct Opal_BufferDesc_t
{
	Opal_BufferUsageFlags usage;
	uint64_t size;
	Opal_AllocationMemoryType memory_type;
	Opal_AllocationHint hint;
} Opal_BufferDesc;

typedef struct Opal_TextureDesc_t
{
	Opal_TextureType type;
	Opal_TextureFormat format;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t mip_count;
	uint32_t layer_count;
	Opal_TextureSamples samples;
	Opal_TextureUsageFlags usage;
	Opal_AllocationHint hint;
} Opal_TextureDesc;

typedef struct Opal_TextureViewDesc_t
{
	Opal_Texture texture;
	Opal_TextureViewType type;
	uint32_t base_mip;
	uint32_t mip_count;
	uint32_t base_layer;
	uint32_t layer_count;
} Opal_TextureViewDesc;

// Function pointers
typedef Opal_Result (*PFN_opalCreateInstance)(Opal_Api api, const Opal_InstanceDesc *desc, Opal_Instance *instance);
typedef Opal_Result (*PFN_opalDestroyInstance)(Opal_Instance instance);

typedef Opal_Result (*PFN_opalEnumerateDevices)(Opal_Instance instance, uint32_t *device_count, Opal_DeviceInfo *infos);

typedef Opal_Result (*PFN_opalCreateDevice)(Opal_Instance instance, uint32_t index, Opal_Device *device);
typedef Opal_Result (*PFN_opalCreateDefaultDevice)(Opal_Instance instance, Opal_DeviceHint hint, Opal_Device *device);
typedef Opal_Result (*PFN_opalDestroyDevice)(Opal_Device device);
typedef Opal_Result (*PFN_opalGetDeviceInfo)(Opal_Device device, Opal_DeviceInfo *info);

typedef Opal_Result (*PFN_opalCreateBuffer)(Opal_Device device, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
typedef Opal_Result (*PFN_opalCreateTexture)(Opal_Device device, const Opal_TextureDesc *desc, Opal_Texture *texture);
typedef Opal_Result (*PFN_opalCreateTextureView)(Opal_Device device, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

typedef Opal_Result (*PFN_opalMapBuffer)(Opal_Device device, Opal_Buffer buffer, void **ptr);
typedef Opal_Result (*PFN_opalUnmapBuffer)(Opal_Device device, Opal_Buffer buffer);

typedef Opal_Result (*PFN_opalDestroyBuffer)(Opal_Device device, Opal_Buffer buffer);
typedef Opal_Result (*PFN_opalDestroyTexture)(Opal_Device device, Opal_Texture texture);
typedef Opal_Result (*PFN_opalDestroyTextureView)(Opal_Device device, Opal_TextureView texture_view);

// API
#if !defined(OPAL_NO_PROTOTYPES)
OPAL_APIENTRY Opal_Result opalCreateInstance(Opal_Api api, const Opal_InstanceDesc *desc, Opal_Instance *instance);
OPAL_APIENTRY Opal_Result opalDestroyInstance(Opal_Instance instance);

OPAL_APIENTRY Opal_Result opalEnumerateDevices(Opal_Instance instance, uint32_t *device_count, Opal_DeviceInfo *infos);

OPAL_APIENTRY Opal_Result opalCreateDevice(Opal_Instance instance, uint32_t index, Opal_Device *device);
OPAL_APIENTRY Opal_Result opalCreateDefaultDevice(Opal_Instance instance, Opal_DeviceHint hint, Opal_Device *device);
OPAL_APIENTRY Opal_Result opalDestroyDevice(Opal_Device device);
OPAL_APIENTRY Opal_Result opalGetDeviceInfo(Opal_Device device, Opal_DeviceInfo *info);

OPAL_APIENTRY Opal_Result opalCreateBuffer(Opal_Device device, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
OPAL_APIENTRY Opal_Result opalCreateTexture(Opal_Device device, const Opal_TextureDesc *desc, Opal_Texture *texture);
OPAL_APIENTRY Opal_Result opalCreateTextureView(Opal_Device device, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);

OPAL_APIENTRY Opal_Result opalMapBuffer(Opal_Device device, Opal_Buffer buffer, void **ptr);
OPAL_APIENTRY Opal_Result opalUnmapBuffer(Opal_Device device, Opal_Buffer buffer);

OPAL_APIENTRY Opal_Result opalDestroyBuffer(Opal_Device device, Opal_Buffer buffer);
OPAL_APIENTRY Opal_Result opalDestroyTexture(Opal_Device device, Opal_Texture texture);
OPAL_APIENTRY Opal_Result opalDestroyTextureView(Opal_Device device, Opal_TextureView texture_view);
#endif

#ifdef __cplusplus
}
#endif
