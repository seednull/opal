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
OPAL_DEFINE_HANDLE(Opal_Surface);
OPAL_DEFINE_HANDLE(Opal_Device);
OPAL_DEFINE_HANDLE(Opal_Queue);
OPAL_DEFINE_HANDLE(Opal_Semaphore);
OPAL_DEFINE_HANDLE(Opal_Buffer);
OPAL_DEFINE_HANDLE(Opal_Texture);
OPAL_DEFINE_HANDLE(Opal_TextureView);
OPAL_DEFINE_HANDLE(Opal_Sampler);
OPAL_DEFINE_HANDLE(Opal_AccelerationStructure);
OPAL_DEFINE_HANDLE(Opal_ShaderBindingTable);
OPAL_DEFINE_HANDLE(Opal_CommandAllocator);
OPAL_DEFINE_HANDLE(Opal_CommandBuffer);
OPAL_DEFINE_HANDLE(Opal_Shader);
OPAL_DEFINE_HANDLE(Opal_DescriptorHeap);
OPAL_DEFINE_HANDLE(Opal_DescriptorSetLayout);
OPAL_DEFINE_HANDLE(Opal_DescriptorSet);
OPAL_DEFINE_HANDLE(Opal_PipelineLayout);
OPAL_DEFINE_HANDLE(Opal_GraphicsPipeline);
OPAL_DEFINE_HANDLE(Opal_ComputePipeline);
OPAL_DEFINE_HANDLE(Opal_RaytracePipeline);
OPAL_DEFINE_HANDLE(Opal_Swapchain);

// Enums
typedef enum Opal_Result_t
{
	OPAL_SUCCESS = 0,
	OPAL_NOT_SUPPORTED,
	OPAL_INVALID_OUTPUT_ARGUMENT,
	OPAL_INVALID_INSTANCE,
	OPAL_INVALID_DEVICE,
	OPAL_INVALID_DEVICE_ENGINE_INDEX,
	OPAL_INVALID_DEVICE_INDEX,
	OPAL_INVALID_QUEUE_INDEX,
	OPAL_INVALID_QUEUE_TYPE,
	OPAL_INVALID_BUFFER,
	OPAL_INVALID_BINDING_INDEX,
	OPAL_NO_MEMORY,
	OPAL_WAIT_TIMEOUT,
	OPAL_SURFACE_NOT_DRAWABLE,
	OPAL_SURFACE_NOT_PRESENTABLE,
	OPAL_SWAPCHAIN_PRESENT_NOT_SUPPORTED,
	OPAL_SWAPCHAIN_PRESENT_MODE_NOT_SUPPORTED,
	OPAL_SWAPCHAIN_FORMAT_NOT_SUPPORTED,
	OPAL_SWAPCHAIN_COLOR_SPACE_NOT_SUPPORTED,
	OPAL_BUFFER_USAGE_NOT_SUPPORTED,
	OPAL_BUFFER_NONMAPPABLE,
	OPAL_TEXTURE_FORMAT_NOT_SUPPORTED,
	OPAL_SHADER_SOURCE_NOT_SUPPORTED,

	// FIXME: add more error codes for internal errors
	OPAL_INTERNAL_ERROR,

	// FIXME: add more error codes for dxgi / d3d12 stuff
	OPAL_DIRECTX12_ERROR,

	// FIXME: add more error codes for webgpu stuff
	OPAL_WEBGPU_ERROR,

	// FIXME: add more error codes for vulkan stuff
	OPAL_VULKAN_ERROR,

	// FIXME: add more error codes for metal stuff
	OPAL_METAL_ERROR,

	OPAL_RESULT_ENUM_MAX,
	OPAL_RESULT_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_Result;

typedef enum Opal_Api_t
{
	OPAL_API_VULKAN = 0,
	OPAL_API_DIRECTX12,
	OPAL_API_METAL,
	OPAL_API_WEBGPU,
	OPAL_API_NULL,

	OPAL_API_AUTO,

	OPAL_API_ENUM_MAX,
	OPAL_API_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_Api;

typedef enum Opal_InstanceCreationFlags_t
{
	OPAL_INSTANCE_CREATION_FLAGS_USE_VMA = 0x00000001,
	OPAL_INSTANCE_CREATION_FLAGS_USE_DEBUG_LAYERS = 0x00000002,

	OPAL_INSTANCE_CREATION_FLAGS_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_InstanceCreationFlags;

typedef enum Opal_DeviceHint_t
{
	OPAL_DEVICE_HINT_DEFAULT = 0,
	OPAL_DEVICE_HINT_PREFER_HIGH_PERFORMANCE,
	OPAL_DEVICE_HINT_PREFER_LOW_POWER,

	OPAL_DEVICE_HINT_ENUM_MAX,
	OPAL_DEVICE_HINT_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_DeviceHint;

typedef enum Opal_DeviceType_t
{
	OPAL_DEVICE_TYPE_DISCRETE = 0,
	OPAL_DEVICE_TYPE_INTEGRATED,
	OPAL_DEVICE_TYPE_CPU,
	OPAL_DEVICE_TYPE_EXTERNAL,
	OPAL_DEVICE_TYPE_UNKNOWN,

	OPAL_DEVICE_TYPE_ENUM_MAX,
	OPAL_DEVICE_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_DeviceType;

typedef enum Opal_DeviceEngineType_t
{
	OPAL_DEVICE_ENGINE_TYPE_MAIN = 0,
	OPAL_DEVICE_ENGINE_TYPE_COMPUTE,
	OPAL_DEVICE_ENGINE_TYPE_COPY,

	OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX,
	OPAL_DEVICE_ENGINE_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_DeviceEngineType;

typedef enum Opal_SemaphoreCreationFlags_t
{
	OPAL_SEMAPHORE_CREATION_FLAGS_HOST_OPERATIONS = 0x00000001,

	OPAL_SEMAPHORE_CREATION_FLAGS_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_SemaphoreCreationFlags;

typedef enum Opal_AllocationMemoryType_t
{
	OPAL_ALLOCATION_MEMORY_TYPE_DEVICE_LOCAL = 0,
	OPAL_ALLOCATION_MEMORY_TYPE_STREAM,
	OPAL_ALLOCATION_MEMORY_TYPE_UPLOAD,
	OPAL_ALLOCATION_MEMORY_TYPE_READBACK,

	OPAL_ALLOCATION_MEMORY_TYPE_ENUM_MAX,
	OPAL_ALLOCATION_MEMORY_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_AllocationMemoryType;

typedef enum Opal_AllocationHint_t
{
	OPAL_ALLOCATION_HINT_AUTO,
	OPAL_ALLOCATION_HINT_PREFER_DEDICATED,
	OPAL_ALLOCATION_HINT_PREFER_HEAP,

	OPAL_ALLOCATION_HINT_ENUM_MAX,
	OPAL_ALLOCATION_HINT_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_AllocationHint;

// TODO: think about either removing this and using Opal_BufferUsageFlags / Opal_ImageUsageFlags directly in barriers
//       or keep only Opal_ResourceState and use it in opalCreateBuffer / opalCreateImage
typedef enum Opal_ResourceState_t
{
	OPAL_RESOURCE_STATE_COMMON = 0x00000000,
	OPAL_RESOURCE_STATE_VERTEX_AND_UNIFORM_BUFFER = 0x00000001,
	OPAL_RESOURCE_STATE_INDEX_BUFFER = 0x00000002,
	OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT = 0x00000004,
	OPAL_RESOURCE_STATE_UNORDERED_ACCESS = 0x00000008,
	OPAL_RESOURCE_STATE_DEPTH_STENCIL_WRITE = 0x00000010,
	OPAL_RESOURCE_STATE_DEPTH_STENCIL_READ = 0x00000020,
	OPAL_RESOURCE_STATE_NON_FRAGMENT_SHADER_RESOURCE = 0x00000040,
	OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE = 0x00000080,
	OPAL_RESOURCE_STATE_COPY_DEST = 0x00000100,
	OPAL_RESOURCE_STATE_COPY_SOURCE = 0x00000200,
	OPAL_RESOURCE_STATE_RESOLVE_DEST = 0x00000400,
	OPAL_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x00000800,
	OPAL_RESOURCE_STATE_PRESENT = 0x00001000,

	OPAL_RESOURCE_STATE_GENERIC_READ = (
		OPAL_RESOURCE_STATE_VERTEX_AND_UNIFORM_BUFFER
		| OPAL_RESOURCE_STATE_INDEX_BUFFER
		| OPAL_RESOURCE_STATE_NON_FRAGMENT_SHADER_RESOURCE
		| OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE
		| OPAL_RESOURCE_STATE_COPY_SOURCE
	),

	OPAL_RESOURCE_STATE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_ResourceState;

typedef enum Opal_AccelerationStructureType_t
{
	OPAL_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL = 0,
	OPAL_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,

	OPAL_ACCELERATION_STRUCTURE_TYPE_ENUM_MAX,
	OPAL_ACCELERATION_STRUCTURE_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_AccelerationStructureType;

typedef enum Opal_AccelerationStructureGeometryType_t
{
	OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_TRIANGLES = 0,
	OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_AABBS,

	OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_ENUM_MAX,
	OPAL_ACCELERATION_STRUCTURE_GEOMETRY_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_AccelerationStructureGeometryType;

typedef enum Opal_AccelerationStructureGeometryFlags_t
{
	OPAL_ACCELERATION_STRUCTURE_GEOMETRY_FLAGS_NONE = 0x00000000,
	OPAL_ACCELERATION_STRUCTURE_GEOMETRY_FLAGS_OPAQUE = 0x00000001,
	OPAL_ACCELERATION_STRUCTURE_GEOMETRY_FLAGS_NO_DUPLICATE_ANYHIT = 0x00000002,

	OPAL_ACCELERATION_STRUCTURE_GEOMETRY_FLAGS_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_AccelerationStructureGeometryFlags;

typedef enum Opal_AccelerationStructureInstanceFlags_t
{
	OPAL_ACCELERATION_STRUCTURE_INSTANCE_FLAGS_NONE = 0x00000000,
	OPAL_ACCELERATION_STRUCTURE_INSTANCE_FLAGS_TRIANGLE_CULL_DISABLE = 0x00000001,
	OPAL_ACCELERATION_STRUCTURE_INSTANCE_FLAGS_TRIANGLE_FRONT_COUNTER_CLOCKWISE = 0x00000002,
	OPAL_ACCELERATION_STRUCTURE_INSTANCE_FLAGS_FORCE_OPAQUE = 0x00000004,
	OPAL_ACCELERATION_STRUCTURE_INSTANCE_FLAGS_FORCE_NO_OPAQUE = 0x00000008,

	OPAL_ACCELERATION_STRUCTURE_INSTANCE_FLAGS_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_AccelerationStructureInstanceFlags;

typedef enum Opal_AccelerationStructureBuildFlags_t
{
	OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_NONE = 0x00000000,
	OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_ALLOW_UPDATE = 0x00000001,
	OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_ALLOW_COMPACTION = 0x00000002,
	OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_PREFER_FAST_TRACE = 0x00000004,
	OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_PREFER_FAST_BUILD = 0x00000008,
	OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_MINIMIZE_MEMORY = 0x00000010,

	OPAL_ACCELERATION_STRUCTURE_BUILD_FLAGS_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_AccelerationStructureBuildFlags;

typedef enum Opal_AccelerationStructureBuildMode_t
{
	OPAL_ACCELERATION_STRUCTURE_BUILD_MODE_BUILD = 0,
	OPAL_ACCELERATION_STRUCTURE_BUILD_MODE_UPDATE,

	OPAL_ACCELERATION_STRUCTURE_BUILD_MODE_ENUM_MAX,
	OPAL_ACCELERATION_STRUCTURE_BUILD_MODE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_AccelerationStructureBuildMode;

typedef enum Opal_AccelerationStructureCopyMode_t
{
	OPAL_ACCELERATION_STRUCTURE_COPY_MODE_CLONE = 0,
	OPAL_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT,

	OPAL_ACCELERATION_STRUCTURE_COPY_MODE_ENUM_MAX,
	OPAL_ACCELERATION_STRUCTURE_COPY_MODE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_AccelerationStructureCopyMode;

typedef enum Opal_BufferUsageFlags_t
{
	OPAL_BUFFER_USAGE_TRANSFER_SRC = 0x00000001,
	OPAL_BUFFER_USAGE_TRANSFER_DST = 0x00000002,
	OPAL_BUFFER_USAGE_VERTEX = 0x00000004,
	OPAL_BUFFER_USAGE_INDEX = 0x00000008,
	OPAL_BUFFER_USAGE_UNIFORM = 0x00000010,
	OPAL_BUFFER_USAGE_STORAGE = 0x00000020,
	OPAL_BUFFER_USAGE_INDIRECT = 0x00000040,
	OPAL_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT = 0x00000080,

	OPAL_BUFFER_USAGE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_BufferUsageFlags;

typedef enum Opal_TextureUsageFlags_t
{
	OPAL_TEXTURE_USAGE_TRANSFER_SRC = 0x00000001,
	OPAL_TEXTURE_USAGE_TRANSFER_DST = 0x00000002,
	OPAL_TEXTURE_USAGE_SHADER_SAMPLED = 0x00000004,
	OPAL_TEXTURE_USAGE_UNORDERED_ACCESS = 0x00000008,
	OPAL_TEXTURE_USAGE_FRAMEBUFFER_ATTACHMENT = 0x00000010,

	OPAL_TEXTURE_USAGE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_TextureUsageFlags;

typedef enum Opal_TextureType_t
{
	OPAL_TEXTURE_TYPE_1D = 0,
	OPAL_TEXTURE_TYPE_2D,
	OPAL_TEXTURE_TYPE_3D,

	OPAL_TEXTURE_TYPE_ENUM_MAX,
	OPAL_TEXTURE_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_TextureType;

typedef enum Opal_TextureViewType_t
{
	OPAL_TEXTURE_VIEW_TYPE_1D = 0,
	OPAL_TEXTURE_VIEW_TYPE_2D,
	OPAL_TEXTURE_VIEW_TYPE_2D_ARRAY,
	OPAL_TEXTURE_VIEW_TYPE_CUBE,
	OPAL_TEXTURE_VIEW_TYPE_CUBE_ARRAY,
	OPAL_TEXTURE_VIEW_TYPE_3D,

	OPAL_TEXTURE_VIEW_TYPE_ENUM_MAX,
	OPAL_TEXTURE_VIEW_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_TextureViewType;

typedef enum Opal_Samples_t
{
	OPAL_SAMPLES_1 = 0,
	OPAL_SAMPLES_2,
	OPAL_SAMPLES_4,
	OPAL_SAMPLES_8,
	OPAL_SAMPLES_16,
	OPAL_SAMPLES_32,
	OPAL_SAMPLES_64,

	OPAL_SAMPLES_ENUM_MAX,
	OPAL_SAMPLES_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_Samples;

typedef enum Opal_PresentMode_t
{
	OPAL_PRESENT_MODE_IMMEDIATE = 0,
	OPAL_PRESENT_MODE_FIFO,
	OPAL_PRESENT_MODE_MAILBOX,

	OPAL_PRESENT_MODE_ENUM_MAX,
	OPAL_PRESENT_MODE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_PresentMode;

typedef enum Opal_ColorSpace_t
{
	OPAL_COLOR_SPACE_SRGB = 0,

	OPAL_COLOR_SPACE_ENUM_MAX,
	OPAL_COLOR_SPACE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_ColorSpace;

typedef enum Opal_TextureFormat_t
{
	OPAL_TEXTURE_FORMAT_UNDEFINED = 0,

	OPAL_TEXTURE_FORMAT_R8_UNORM,
	OPAL_TEXTURE_FORMAT_R8_SNORM,
	OPAL_TEXTURE_FORMAT_R8_UINT,
	OPAL_TEXTURE_FORMAT_R8_SINT,

	OPAL_TEXTURE_FORMAT_RG8_UNORM,
	OPAL_TEXTURE_FORMAT_RG8_SNORM,
	OPAL_TEXTURE_FORMAT_RG8_UINT,
	OPAL_TEXTURE_FORMAT_RG8_SINT,

	OPAL_TEXTURE_FORMAT_RGBA8_UNORM,
	OPAL_TEXTURE_FORMAT_RGBA8_SNORM,
	OPAL_TEXTURE_FORMAT_RGBA8_UINT,
	OPAL_TEXTURE_FORMAT_RGBA8_SINT,

	OPAL_TEXTURE_FORMAT_R16_UINT,
	OPAL_TEXTURE_FORMAT_R16_SINT,
	OPAL_TEXTURE_FORMAT_R16_SFLOAT,

	OPAL_TEXTURE_FORMAT_RG16_UINT,
	OPAL_TEXTURE_FORMAT_RG16_SINT,
	OPAL_TEXTURE_FORMAT_RG16_SFLOAT,

	OPAL_TEXTURE_FORMAT_RGBA16_UINT,
	OPAL_TEXTURE_FORMAT_RGBA16_SINT,
	OPAL_TEXTURE_FORMAT_RGBA16_SFLOAT,

	OPAL_TEXTURE_FORMAT_R32_UINT,
	OPAL_TEXTURE_FORMAT_R32_SINT,
	OPAL_TEXTURE_FORMAT_R32_SFLOAT,

	OPAL_TEXTURE_FORMAT_RG32_UINT,
	OPAL_TEXTURE_FORMAT_RG32_SINT,
	OPAL_TEXTURE_FORMAT_RG32_SFLOAT,

	OPAL_TEXTURE_FORMAT_RGBA32_UINT,
	OPAL_TEXTURE_FORMAT_RGBA32_SINT,
	OPAL_TEXTURE_FORMAT_RGBA32_SFLOAT,

	OPAL_TEXTURE_FORMAT_BGRA8_UNORM,
	OPAL_TEXTURE_FORMAT_BGRA8_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_RGBA8_UNORM_SRGB,

	OPAL_TEXTURE_FORMAT_RG11B10_UFLOAT,
	OPAL_TEXTURE_FORMAT_RGB9E5_UFLOAT,

	OPAL_TEXTURE_FORMAT_BC1_R5G6B5_UNORM,
	OPAL_TEXTURE_FORMAT_BC1_R5G6B5_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_BC1_R5G6B5A1_UNORM,
	OPAL_TEXTURE_FORMAT_BC1_R5G6B5A1_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_BC2_R5G6B5A4_UNORM,
	OPAL_TEXTURE_FORMAT_BC2_R5G6B5A4_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_BC3_RGBA8_UNORM,
	OPAL_TEXTURE_FORMAT_BC3_RGBA8_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_BC4_R8_UNORM,
	OPAL_TEXTURE_FORMAT_BC4_R8_SNORM,
	OPAL_TEXTURE_FORMAT_BC5_RG8_UNORM,
	OPAL_TEXTURE_FORMAT_BC5_RG8_SNORM,
	OPAL_TEXTURE_FORMAT_BC6H_RGB16_UFLOAT,
	OPAL_TEXTURE_FORMAT_BC6H_RGB16_SFLOAT,
	OPAL_TEXTURE_FORMAT_BC7_RGBA8_UNORM,
	OPAL_TEXTURE_FORMAT_BC7_RGBA8_UNORM_SRGB,

	OPAL_TEXTURE_FORMAT_ETC2_RGB8_UNORM,
	OPAL_TEXTURE_FORMAT_ETC2_RGB8_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ETC2_RGB8A1_UNORM,
	OPAL_TEXTURE_FORMAT_ETC2_RGB8A1_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ETC2_RGBA8_UNORM,
	OPAL_TEXTURE_FORMAT_ETC2_RGBA8_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_EAC_R11_UNORM,
	OPAL_TEXTURE_FORMAT_EAC_R11_SNORM,
	OPAL_TEXTURE_FORMAT_EAC_RG11_UNORM,
	OPAL_TEXTURE_FORMAT_EAC_RG11_SNORM,

	OPAL_TEXTURE_FORMAT_ASTC_4x4_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_4x4_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_5x4_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_5x4_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_5x5_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_5x5_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_6x5_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_6x5_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_6x6_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_6x6_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_8x5_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_8x5_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_8x6_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_8x6_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_8x8_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_8x8_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_10x5_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_10x5_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_10x6_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_10x6_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_10x8_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_10x8_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_10x10_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_10x10_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_12x10_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_12x10_UNORM_SRGB,
	OPAL_TEXTURE_FORMAT_ASTC_12x12_UNORM,
	OPAL_TEXTURE_FORMAT_ASTC_12x12_UNORM_SRGB,

	OPAL_TEXTURE_FORMAT_D16_UNORM,
	OPAL_TEXTURE_FORMAT_D32_SFLOAT,
	OPAL_TEXTURE_FORMAT_D16_UNORM_S8_UINT,
	OPAL_TEXTURE_FORMAT_D24_UNORM_S8_UINT,
	OPAL_TEXTURE_FORMAT_D32_SFLOAT_S8_UINT,

	OPAL_TEXTURE_FORMAT_COLOR_BEGIN = OPAL_TEXTURE_FORMAT_R8_UNORM,
	OPAL_TEXTURE_FORMAT_COLOR_END = OPAL_TEXTURE_FORMAT_ASTC_12x12_UNORM_SRGB,

	OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_BEGIN = OPAL_TEXTURE_FORMAT_D16_UNORM,
	OPAL_TEXTURE_FORMAT_DEPTH_STENCIL_END = OPAL_TEXTURE_FORMAT_D32_SFLOAT_S8_UINT,

	OPAL_TEXTURE_FORMAT_ENUM_MAX,
	OPAL_TEXTURE_FORMAT_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_TextureFormat;

typedef enum Opal_VertexFormat_t
{
	OPAL_VERTEX_FORMAT_UNDEFINED = 0,

	OPAL_VERTEX_FORMAT_RG8_UNORM,
	OPAL_VERTEX_FORMAT_RG8_SNORM,
	OPAL_VERTEX_FORMAT_RG8_UINT,
	OPAL_VERTEX_FORMAT_RG8_SINT,

	OPAL_VERTEX_FORMAT_RGBA8_UNORM,
	OPAL_VERTEX_FORMAT_RGBA8_SNORM,
	OPAL_VERTEX_FORMAT_RGBA8_UINT,
	OPAL_VERTEX_FORMAT_RGBA8_SINT,

	OPAL_VERTEX_FORMAT_RG16_UNORM,
	OPAL_VERTEX_FORMAT_RG16_SNORM,
	OPAL_VERTEX_FORMAT_RG16_UINT,
	OPAL_VERTEX_FORMAT_RG16_SINT,
	OPAL_VERTEX_FORMAT_RG16_SFLOAT,

	OPAL_VERTEX_FORMAT_RGBA16_UNORM,
	OPAL_VERTEX_FORMAT_RGBA16_SNORM,
	OPAL_VERTEX_FORMAT_RGBA16_UINT,
	OPAL_VERTEX_FORMAT_RGBA16_SINT,
	OPAL_VERTEX_FORMAT_RGBA16_SFLOAT,

	OPAL_VERTEX_FORMAT_R32_UINT,
	OPAL_VERTEX_FORMAT_R32_SINT,
	OPAL_VERTEX_FORMAT_R32_SFLOAT,

	OPAL_VERTEX_FORMAT_RG32_UINT,
	OPAL_VERTEX_FORMAT_RG32_SINT,
	OPAL_VERTEX_FORMAT_RG32_SFLOAT,

	OPAL_VERTEX_FORMAT_RGB32_UINT,
	OPAL_VERTEX_FORMAT_RGB32_SINT,
	OPAL_VERTEX_FORMAT_RGB32_SFLOAT,

	OPAL_VERTEX_FORMAT_RGBA32_UINT,
	OPAL_VERTEX_FORMAT_RGBA32_SINT,
	OPAL_VERTEX_FORMAT_RGBA32_SFLOAT,

	OPAL_VERTEX_FORMAT_RGB10A2_UNORM,

	OPAL_VERTEX_FORMAT_ENUM_MAX,
	OPAL_VERTEX_FORMAT_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_VertexFormat;

typedef enum Opal_IndexFormat_t
{
	OPAL_INDEX_FORMAT_UNDEFINED = 0,

	OPAL_INDEX_FORMAT_UINT16,
	OPAL_INDEX_FORMAT_UINT32,

	OPAL_INDEX_FORMAT_ENUM_MAX,
	OPAL_INDEX_FORMAT_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_IndexFormat;

typedef enum Opal_SamplerFilterMode_t
{
	OPAL_SAMPLER_FILTER_MODE_NEAREST = 0,
	OPAL_SAMPLER_FILTER_MODE_LINEAR,

	OPAL_SAMPLER_FILTER_MODE_ENUM_MAX,
	OPAL_SAMPLER_FILTER_MODE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_SamplerFilterMode;

typedef enum Opal_SamplerAddressMode_t
{
	OPAL_SAMPLER_ADDRESS_MODE_REPEAT = 0,
	OPAL_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
	OPAL_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,

	OPAL_SAMPLER_ADDRESS_MODE_ENUM_MAX,
	OPAL_SAMPLER_ADDRESS_MODE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_SamplerAddressMode;

typedef enum Opal_ShaderSourceType_t
{
	OPAL_SHADER_SOURCE_TYPE_SPIRV_BINARY = 0,
	OPAL_SHADER_SOURCE_TYPE_DXIL_BINARY,
	OPAL_SHADER_SOURCE_TYPE_METALLIB_BINARY,
	OPAL_SHADER_SOURCE_TYPE_WGSL_SOURCE,

	OPAL_SHADER_SOURCE_ENUM_MAX,
	OPAL_SHADER_SOURCE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_ShaderSourceType;

typedef enum Opal_LoadOp_t
{
	OPAL_LOAD_OP_DONT_CARE = 0,
	OPAL_LOAD_OP_CLEAR,
	OPAL_LOAD_OP_LOAD,

	OPAL_LOAD_OP_ENUM_MAX,
	OPAL_LOAD_OP_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_LoadOp;

typedef enum Opal_StoreOp_t
{
	OPAL_STORE_OP_DONT_CARE = 0,
	OPAL_STORE_OP_STORE,

	OPAL_STORE_OP_ENUM_MAX,
	OPAL_STORE_OP_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_StoreOp;

typedef enum Opal_CompareOp_t {
	OPAL_COMPARE_OP_NEVER = 0,
	OPAL_COMPARE_OP_LESS,
	OPAL_COMPARE_OP_EQUAL,
	OPAL_COMPARE_OP_LESS_OR_EQUAL,
	OPAL_COMPARE_OP_GREATER,
	OPAL_COMPARE_OP_NOT_EQUAL,
	OPAL_COMPARE_OP_GREATER_OR_EQUAL,
	OPAL_COMPARE_OP_ALWAYS,

	OPAL_COMPARE_OP_ENUM_MAX,
	OPAL_COMPARE_OP_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_CompareOp;

typedef enum Opal_StencilOp_t {
	OPAL_STENCIL_OP_KEEP = 0,
	OPAL_STENCIL_OP_ZERO,
	OPAL_STENCIL_OP_REPLACE,
	OPAL_STENCIL_OP_INVERT,
	OPAL_STENCIL_OP_INCREMENT_AND_CLAMP,
	OPAL_STENCIL_OP_DECREMENT_AND_CLAMP,
	OPAL_STENCIL_OP_INCREMENT_AND_WRAP,
	OPAL_STENCIL_OP_DECREMENT_AND_WRAP,

	OPAL_STENCIL_OP_ENUM_MAX,
	OPAL_STENCIL_OP_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_StencilOp;

typedef enum Opal_BlendOp_t {
	OPAL_BLEND_OP_ADD = 0,
	OPAL_BLEND_OP_SUBTRACT,
	OPAL_BLEND_OP_REVERSE_SUBTRACT,
	OPAL_BLEND_OP_MIN,
	OPAL_BLEND_OP_MAX,

	OPAL_BLEND_OP_ENUM_MAX,
	OPAL_BLEND_OP_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_BlendOp;

typedef enum Opal_BlendFactor_t {
	OPAL_BLEND_FACTOR_ZERO = 0,
	OPAL_BLEND_FACTOR_ONE,
	OPAL_BLEND_FACTOR_SRC_COLOR,
	OPAL_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	OPAL_BLEND_FACTOR_DST_COLOR,
	OPAL_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	OPAL_BLEND_FACTOR_SRC_ALPHA,
	OPAL_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	OPAL_BLEND_FACTOR_DST_ALPHA,
	OPAL_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,

	OPAL_BLEND_FACTOR_ENUM_MAX,
	OPAL_BLEND_FACTOR_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_BlendFactor;

typedef enum Opal_VertexInputRate_t
{
	OPAL_VERTEX_INPUT_RATE_VERTEX = 0,
	OPAL_VERTEX_INPUT_RATE_INSTANCE,

	OPAL_VERTEX_INPUT_RATE_ENUM_MAX,
	OPAL_VERTEX_INPUT_RATE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_VertexInputRate;

typedef enum Opal_ShaderStage_t
{
	OPAL_SHADER_STAGE_NONE = 0x00000000,
	OPAL_SHADER_STAGE_VERTEX = 0x00000001,
	OPAL_SHADER_STAGE_TESSELLATION_CONTROL = 0x00000002,
	OPAL_SHADER_STAGE_TESSELLATION_EVALUATION = 0x00000004,
	OPAL_SHADER_STAGE_GEOMETRY = 0x00000008,
	OPAL_SHADER_STAGE_FRAGMENT = 0x00000010,
	OPAL_SHADER_STAGE_COMPUTE = 0x00000020,

	OPAL_SHADER_STAGE_TASK = 0x00000040,
	OPAL_SHADER_STAGE_MESH = 0x00000080,

	OPAL_SHADER_STAGE_RAYGEN = 0x00000100,
	OPAL_SHADER_STAGE_ANY_HIT = 0x00000200,
	OPAL_SHADER_STAGE_CLOSEST_HIT = 0x00000400,
	OPAL_SHADER_STAGE_MISS = 0x00000800,
	OPAL_SHADER_STAGE_INTERSECTION = 0x00001000,

	OPAL_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
	OPAL_SHADER_STAGE_ALL_MESHLET = 0x000000C0,
	OPAL_SHADER_STAGE_ALL_RAYTRACE = 0x00001F00,
	OPAL_SHADER_STAGE_ALL = 0x7FFFFFFF,

	OPAL_SHADER_STAGE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_ShaderStage;

typedef enum Opal_DescriptorType_t
{
	OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 0,
	OPAL_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
	OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER,
	OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
	OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY,
	OPAL_DESCRIPTOR_TYPE_STORAGE_BUFFER_READONLY_DYNAMIC,
	OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_1D,
	OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_2D,
	OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_2D_ARRAY,
	OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_CUBE,
	OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_CUBE_ARRAY,
	OPAL_DESCRIPTOR_TYPE_SAMPLED_TEXTURE_3D,
	OPAL_DESCRIPTOR_TYPE_MULTISAMPLED_TEXTURE_2D,
	OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_1D,
	OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_2D,
	OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_2D_ARRAY,
	OPAL_DESCRIPTOR_TYPE_STORAGE_TEXTURE_3D,
	OPAL_DESCRIPTOR_TYPE_SAMPLER,
	OPAL_DESCRIPTOR_TYPE_COMPARE_SAMPLER,
	OPAL_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE,

	OPAL_DESCRIPTOR_TYPE_ENUM_MAX,
	OPAL_DESCRIPTOR_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_DescriptorType;

typedef enum Opal_PrimitiveType_t {
	OPAL_PRIMITIVE_TYPE_POINT_LIST = 0,
	OPAL_PRIMITIVE_TYPE_LINE_LIST,
	OPAL_PRIMITIVE_TYPE_LINE_STRIP,
	OPAL_PRIMITIVE_TYPE_TRIANGLE_LIST,
	OPAL_PRIMITIVE_TYPE_TRIANGLE_STRIP,
	OPAL_PRIMITIVE_TYPE_TRIANGLE_PATCH,
	OPAL_PRIMITIVE_TYPE_QUAD_PATCH,

	OPAL_PRIMITIVE_TYPE_ENUM_MAX,
	OPAL_PRIMITIVE_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_PrimitiveType;

typedef enum Opal_CullMode_t {
	OPAL_CULL_MODE_NONE = 0,
	OPAL_CULL_MODE_FRONT,
	OPAL_CULL_MODE_BACK,

	OPAL_CULL_MODE_ENUM_MAX,
	OPAL_CULL_MODE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_CullMode;

typedef enum Opal_FrontFace_t {
	OPAL_FRONT_FACE_COUNTER_CLOCKWISE = 0,
	OPAL_FRONT_FACE_CLOCKWISE,

	OPAL_FRONT_FACE_ENUM_MAX,
	OPAL_FRONT_FACE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_FrontFace;

// Structs
typedef struct Opal_DeviceLimits_t
{
	uint32_t max_texture_dimension_1d;
	uint32_t max_texture_dimension_2d;
	uint32_t max_texture_dimension_3d;
	uint32_t max_texture_array_layers;
	uint64_t max_buffer_size;
	uint64_t min_uniform_buffer_offset_alignment;
	uint64_t min_storage_buffer_offset_alignment;
	uint32_t max_descriptor_sets;
	uint64_t max_uniform_buffer_binding_size;
	uint64_t max_storage_buffer_binding_size;
	uint32_t max_vertex_buffers;
	uint32_t max_vertex_attributes;
	uint32_t max_vertex_buffer_stride;
	uint32_t max_color_attachments;
	uint32_t max_compute_shared_memory_size;
	uint32_t max_compute_workgroup_count_x;
	uint32_t max_compute_workgroup_count_y;
	uint32_t max_compute_workgroup_count_z;
	uint32_t max_compute_workgroup_invocations;
	uint32_t max_compute_workgroup_local_size_x;
	uint32_t max_compute_workgroup_local_size_y;
	uint32_t max_compute_workgroup_local_size_z;
	uint32_t max_raytrace_recursion_depth;
	uint32_t max_raytrace_hit_attribute_size;
} Opal_DeviceLimits;

typedef struct Opal_DeviceFeatures_t
{
	uint32_t queue_count[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];
	uint32_t acceleration_structure_instance_size;
	uint8_t tessellation_shader;
	uint8_t geometry_shader;
	uint8_t compute_pipeline;
	uint8_t meshlet_pipeline;
	uint8_t raytrace_pipeline;
	uint8_t texture_compression_etc2;
	uint8_t texture_compression_astc;
	uint8_t texture_compression_bc;
} Opal_DeviceFeatures;

typedef struct Opal_DeviceInfo_t
{
	char name[256];
	Opal_Api api;
	Opal_DeviceType device_type;
	uint64_t driver_version;
	uint32_t vendor_id;
	uint32_t device_id;
	Opal_DeviceFeatures features;
	Opal_DeviceLimits limits;
} Opal_DeviceInfo;

typedef struct Opal_BufferView_t
{
	Opal_Buffer buffer;
	uint64_t offset;
	uint64_t size;
} Opal_BufferView;

typedef struct Opal_StorageBufferView_t
{
	Opal_Buffer buffer;
	uint64_t offset;
	uint32_t element_size;
	uint32_t num_elements;
} Opal_StorageBufferView;

typedef struct Opal_VertexBufferView_t
{
	Opal_Buffer buffer;
	uint64_t offset;
	uint32_t size;
	uint32_t stride;
} Opal_VertexBufferView;

typedef struct Opal_IndexBufferView_t
{
	Opal_Buffer buffer;
	uint64_t offset;
	uint32_t size;
	Opal_IndexFormat format;
} Opal_IndexBufferView;

typedef struct Opal_InstanceDesc_t
{
	const char *application_name;
	const char *engine_name;
	uint32_t application_version;
	uint32_t engine_version;
	uint32_t heap_size;
	uint32_t max_heap_allocations;
	uint32_t max_heaps;
	Opal_InstanceCreationFlags flags;
} Opal_InstanceDesc;

typedef struct Opal_SemaphoreDesc_t
{
	uint64_t initial_value;
	Opal_SemaphoreCreationFlags flags;
} Opal_SemaphoreDesc;

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
	Opal_Samples samples;
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

typedef struct Opal_SamplerDesc_t
{
	Opal_SamplerFilterMode mag_filter;
	Opal_SamplerFilterMode min_filter;
	Opal_SamplerFilterMode mip_filter;
	Opal_SamplerAddressMode address_mode_u;
	Opal_SamplerAddressMode address_mode_v;
	Opal_SamplerAddressMode address_mode_w;
	float min_lod;
	float max_lod;
	uint32_t max_anisotropy;
	uint32_t compare_enable;
	Opal_CompareOp compare_op;
} Opal_SamplerDesc;

typedef struct Opal_AccelerationStructureDesc_t
{
	Opal_AccelerationStructureType type;
	uint64_t size;
} Opal_AccelerationStructureDesc;

typedef struct Opal_ShaderDesc_t
{
	Opal_ShaderSourceType type;
	const void *data;
	uint64_t size;
} Opal_ShaderDesc;

typedef struct Opal_ShaderFunction_t
{
	Opal_Shader shader;
	const char *name;
} Opal_ShaderFunction;

typedef union Opal_ClearColor_t
{
	float f[4];
	uint32_t u[4];
	int32_t i[4];
} Opal_ClearColor;

typedef struct Opal_ClearDepthStencil_t
{
	float depth;
	uint32_t stencil;
} Opal_ClearDepthStencil;

typedef union Opal_ClearValue_t
{
	Opal_ClearColor color;
	Opal_ClearDepthStencil depth_stencil;
} Opal_ClearValue;

typedef struct Opal_FramebufferAttachment_t
{
	Opal_TextureView texture_view;
	Opal_TextureView resolve_texture_view;
	Opal_LoadOp load_op;
	Opal_StoreOp store_op;
	Opal_ClearValue clear_value;
} Opal_FramebufferAttachment;

typedef struct Opal_BufferTextureRegion_t
{
	Opal_Buffer buffer;
	uint64_t offset;
	uint32_t row_size;
	uint32_t num_rows;
} Opal_BufferTextureRegion;

typedef struct Opal_Offset3D_t
{
	int32_t x;
	int32_t y;
	int32_t z;
} Opal_Offset3D;

typedef struct Opal_Extent3D_t
{
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} Opal_Extent3D;

typedef struct Opal_TextureRegion_t
{
	Opal_TextureView texture_view;
	Opal_Offset3D offset;
} Opal_TextureRegion;

typedef struct Opal_VertexAttribute_t
{
	Opal_VertexFormat format;
	uint32_t offset;
} Opal_VertexAttribute;

typedef struct Opal_VertexStream_t
{
	uint32_t stride;
	uint32_t num_vertex_attributes;
	const Opal_VertexAttribute *attributes;
	Opal_VertexInputRate rate;
} Opal_VertexStream;

typedef struct Opal_DescriptorSetLayoutEntry_t
{
	uint32_t binding;
	Opal_DescriptorType type;
	Opal_ShaderStage visibility;
	Opal_TextureFormat texture_format;
} Opal_DescriptorSetLayoutEntry;

typedef struct Opal_DescriptorHeapDesc_t
{
	uint32_t num_resource_descriptors;
	uint32_t num_sampler_descriptors;
} Opal_DescriptorHeapDesc;

typedef union Opal_DescriptorSetEntryData_t
{
	Opal_BufferView buffer_view;
	Opal_StorageBufferView storage_buffer_view;
	Opal_TextureView texture_view;
	Opal_Sampler sampler;
	Opal_AccelerationStructure acceleration_structure;
} Opal_DescriptorSetEntryData;

typedef struct Opal_DescriptorSetEntry_t
{
	uint32_t binding;
	Opal_DescriptorSetEntryData data;
} Opal_DescriptorSetEntry;

typedef struct Opal_DescriptorSetAllocationDesc_t
{
	Opal_DescriptorSetLayout layout;
	Opal_DescriptorHeap heap;
	uint32_t num_entries;
	const Opal_DescriptorSetEntry *entries;
} Opal_DescriptorSetAllocationDesc;

typedef struct Opal_Viewport_t
{
	float x;
	float y;
	float width;
	float height;
	float min_depth;
	float max_depth;
} Opal_Viewport;

typedef struct Opal_BlendState_t
{
	uint32_t enable;
	Opal_BlendFactor src_color;
	Opal_BlendFactor dst_color;
	Opal_BlendOp color_op;
	Opal_BlendFactor src_alpha;
	Opal_BlendFactor dst_alpha;
	Opal_BlendOp alpha_op;
} Opal_BlendState;

typedef struct Opal_StencilFaceState_t
{
	Opal_StencilOp pass_op;
	Opal_StencilOp depth_fail_op;
	Opal_StencilOp fail_op;
	Opal_CompareOp compare_op;
} Opal_StencilFaceState;

typedef struct Opal_AccelerationStructureGeometryDataTriangles_t
{
	uint32_t num_vertices;
	uint32_t num_indices;
	Opal_VertexFormat vertex_format;
	Opal_IndexFormat index_format;
	uint32_t vertex_stride;
	Opal_BufferView vertex_buffer;
	Opal_BufferView index_buffer;
} Opal_AccelerationStructureGeometryDataTriangles;

typedef struct Opal_AccelerationStructureGeometryDataAABBs_t
{
	uint32_t num_entries;
	uint32_t stride;
	Opal_BufferView entries_buffer;
} Opal_AccelerationStructureGeometryDataAABBs;

typedef union Opal_AccelerationStructureGeometryData_t
{
	Opal_AccelerationStructureGeometryDataTriangles triangles;
	Opal_AccelerationStructureGeometryDataAABBs aabbs;
} Opal_AccelerationStructureGeometryData;

typedef struct Opal_AccelerationStructureGeometry_t
{
	Opal_AccelerationStructureGeometryType type;
	Opal_AccelerationStructureGeometryFlags flags;
	Opal_AccelerationStructureGeometryData data;
} Opal_AccelerationStructureGeometry;

typedef struct Opal_AccelerationStructureBuildInputBottomLevel_t
{
	uint32_t num_geometries;
	const Opal_AccelerationStructureGeometry *geometries;
} Opal_AccelerationStructureBuildInputBottomLevel;

typedef struct Opal_AccelerationStructureBuildInputTopLevel_t
{
	uint32_t num_instances;
	Opal_BufferView instance_buffer;
} Opal_AccelerationStructureBuildInputTopLevel;

typedef union Opal_AccelerationStructureBuildInput_t
{
	Opal_AccelerationStructureBuildInputBottomLevel bottom_level;
	Opal_AccelerationStructureBuildInputTopLevel top_level;
} Opal_AccelerationStructureBuildInput;

typedef struct Opal_AccelerationStructureBuildDesc_t
{
	Opal_AccelerationStructureType type;
	Opal_AccelerationStructureBuildFlags build_flags;
	Opal_AccelerationStructureBuildMode build_mode;
	Opal_AccelerationStructureBuildInput input;

	Opal_AccelerationStructure src_acceleration_structure;
	Opal_AccelerationStructure dst_acceleration_structure;
	Opal_BufferView scratch_buffer;
} Opal_AccelerationStructureBuildDesc;

typedef struct Opal_AccelerationStructureCopyDesc_t
{
	Opal_AccelerationStructureCopyMode copy_mode;

	Opal_AccelerationStructure src_acceleration_structure;
	Opal_AccelerationStructure dst_acceleration_structure;
} Opal_AccelerationStructureCopyDesc;

typedef struct Opal_AccelerationStructurePrebuildInfo_t
{
	uint64_t acceleration_structure_size;
	uint64_t build_scratch_size;
	uint64_t update_scratch_size;
} Opal_AccelerationStructurePrebuildInfo;

typedef struct Opal_AccelerationStructureInstance_t
{
	float transform[3][4];
	uint32_t custom_index : 24;
	uint32_t mask : 8;
	uint32_t intersection_index_offset : 24;
	Opal_AccelerationStructureInstanceFlags flags : 8;
	Opal_AccelerationStructure blas;
} Opal_AccelerationStructureInstance;

typedef struct Opal_AccelerationStructureInstanceBufferBuildDesc_t
{
	Opal_BufferView buffer;
	uint32_t num_instances;
	const Opal_AccelerationStructureInstance *instances;
} Opal_AccelerationStructureInstanceBufferBuildDesc;

typedef struct Opal_ShaderBindingTableBuildDesc_t
{
	uint32_t num_raygen_indices;
	const uint32_t *raygen_indices;

	uint32_t num_miss_indices;
	const uint32_t *miss_indices;

	uint32_t num_intersection_indices;
	const uint32_t *intersection_indices;
} Opal_ShaderBindingTableBuildDesc;

typedef struct Opal_GraphicsPipelineDesc_t
{
	Opal_PipelineLayout pipeline_layout;

	Opal_ShaderFunction vertex_function;
	Opal_ShaderFunction tessellation_control_function;
	Opal_ShaderFunction tessellation_evaluation_function;
	Opal_ShaderFunction geometry_function;
	Opal_ShaderFunction fragment_function;

	uint32_t num_vertex_streams;
	const Opal_VertexStream *vertex_streams;
	Opal_PrimitiveType primitive_type;
	Opal_IndexFormat strip_index_format;

	Opal_CullMode cull_mode;
	Opal_FrontFace front_face;
	Opal_Samples rasterization_samples;

	uint32_t depth_enable;
	uint32_t depth_write;
	Opal_CompareOp depth_compare_op;

	uint32_t stencil_enable;
	Opal_StencilFaceState stencil_front;
	Opal_StencilFaceState stencil_back;
	uint32_t stencil_read_mask;
	uint32_t stencil_write_mask;

	uint32_t num_color_attachments;
	Opal_TextureFormat color_attachment_formats[8];
	Opal_BlendState color_blend_states[8];
	// TODO: color write masks

	Opal_TextureFormat *depth_stencil_attachment_format;
} Opal_GraphicsPipelineDesc;

typedef struct Opal_MeshletPipelineDesc_t
{
	Opal_PipelineLayout pipeline_layout;

	Opal_ShaderFunction task_function;
	Opal_ShaderFunction mesh_function;
	Opal_ShaderFunction fragment_function;

	Opal_CullMode cull_mode;
	Opal_FrontFace front_face;
	Opal_Samples rasterization_samples;

	uint32_t depth_enable;
	uint32_t depth_write;
	Opal_CompareOp depth_compare_op;

	uint32_t stencil_enable;
	Opal_StencilFaceState stentil_front;
	Opal_StencilFaceState stentil_back;
	uint32_t stencil_read_mask;
	uint32_t stencil_write_mask;

	uint32_t num_color_attachments;
	Opal_TextureFormat color_attachment_formats[8];
	Opal_BlendState color_blend_states[8];
	// TODO: color write masks

	Opal_TextureFormat *depth_stencil_attachment_format;
} Opal_MeshletPipelineDesc;

typedef struct Opal_ComputePipelineDesc_t
{
	Opal_PipelineLayout pipeline_layout;
	Opal_ShaderFunction compute_function;
	uint32_t threadgroup_size_x;
	uint32_t threadgroup_size_y;
	uint32_t threadgroup_size_z;
} Opal_ComputePipelineDesc;

typedef struct Opal_ShaderIntersectionGroup_t
{
	Opal_ShaderFunction intersection_function;
	Opal_ShaderFunction anyhit_function;
	Opal_ShaderFunction closesthit_function;
} Opal_ShaderIntersectionGroup;

typedef struct Opal_RaytracePipelineDesc_t
{
	Opal_PipelineLayout pipeline_layout;

	uint32_t num_raygen_functions;
	const Opal_ShaderFunction *raygen_functions;

	uint32_t num_miss_functions;
	const Opal_ShaderFunction *miss_functions;

	uint32_t num_intersection_functions;
	const Opal_ShaderIntersectionGroup *intersection_functions;

	uint32_t max_recursion_depth;
	uint32_t max_ray_payload_size;
	uint32_t max_hit_attribute_size;
} Opal_RaytracePipelineDesc;

typedef struct Opal_SurfaceFormat_t
{
	Opal_TextureFormat texture_format;
	Opal_ColorSpace color_space;
} Opal_SurfaceFormat;

typedef struct Opal_SwapchainDesc_t
{
	Opal_PresentMode mode;
	Opal_SurfaceFormat format;
	Opal_TextureUsageFlags usage;
	Opal_Surface surface;
} Opal_SwapchainDesc;

typedef struct Opal_SubmitDesc_t
{
	uint32_t num_wait_semaphores;
	const Opal_Semaphore *wait_semaphores;
	const uint64_t *wait_values;

	uint32_t num_wait_swapchains;
	const Opal_Swapchain *wait_swapchains;

	uint32_t num_command_buffers;
	const Opal_CommandBuffer *command_buffers;

	uint32_t num_signal_semaphores;
	const Opal_Semaphore *signal_semaphores;
	const uint64_t *signal_values;

	uint32_t num_signal_swapchains;
	const Opal_Swapchain *signal_swapchains;
} Opal_SubmitDesc;

// Function pointers
typedef Opal_Result (*PFN_opalEnumerateDevices)(Opal_Instance instance, uint32_t *device_count, Opal_DeviceInfo *infos);

typedef Opal_Result (*PFN_opalCreateSurface)(Opal_Instance instance, void *handle, Opal_Surface *surface);
typedef Opal_Result (*PFN_opalCreateDevice)(Opal_Instance instance, uint32_t index, Opal_Device *device);
typedef Opal_Result (*PFN_opalCreateDefaultDevice)(Opal_Instance instance, Opal_DeviceHint hint, Opal_Device *device);

typedef Opal_Result (*PFN_opalDestroySurface)(Opal_Instance instance, Opal_Surface surface);
typedef Opal_Result (*PFN_opalDestroyInstance)(Opal_Instance instance);

typedef Opal_Result (*PFN_opalGetDeviceInfo)(Opal_Device device, Opal_DeviceInfo *info);
typedef Opal_Result (*PFN_opalGetDeviceQueue)(Opal_Device device, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue);
typedef Opal_Result (*PFN_opalGetAccelerationStructurePrebuildInfo)(Opal_Device device, const Opal_AccelerationStructureBuildDesc *desc, Opal_AccelerationStructurePrebuildInfo *info);
typedef Opal_Result (*PFN_opalGetSupportedSurfaceFormats)(Opal_Device device, Opal_Surface surface, uint32_t *num_formats, Opal_SurfaceFormat *formats);
typedef Opal_Result (*PFN_opalGetSupportedPresentModes)(Opal_Device device, Opal_Surface surface, uint32_t *num_present_modes, Opal_PresentMode *present_modes);
typedef Opal_Result (*PFN_opalGetPreferredSurfaceFormat)(Opal_Device device, Opal_Surface surface, Opal_SurfaceFormat *format);
typedef Opal_Result (*PFN_opalGetPreferredSurfacePresentMode)(Opal_Device device, Opal_Surface surface, Opal_PresentMode *present_mode);

typedef Opal_Result (*PFN_opalCreateSemaphore)(Opal_Device device, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore);
typedef Opal_Result (*PFN_opalCreateBuffer)(Opal_Device device, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
typedef Opal_Result (*PFN_opalCreateTexture)(Opal_Device device, const Opal_TextureDesc *desc, Opal_Texture *texture);
typedef Opal_Result (*PFN_opalCreateTextureView)(Opal_Device device, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);
typedef Opal_Result (*PFN_opalCreateSampler)(Opal_Device device, const Opal_SamplerDesc *desc, Opal_Sampler *sampler);
typedef Opal_Result (*PFN_opalCreateAccelerationStructure)(Opal_Device device, const Opal_AccelerationStructureDesc *desc, Opal_AccelerationStructure *acceleration_structure);
typedef Opal_Result (*PFN_opalCreateShaderBindingTable)(Opal_Device device, Opal_RaytracePipeline pipeline, Opal_ShaderBindingTable *shader_binding_table);
typedef Opal_Result (*PFN_opalCreateCommandAllocator)(Opal_Device device, Opal_Queue queue, Opal_CommandAllocator *command_allocator);
typedef Opal_Result (*PFN_opalCreateCommandBuffer)(Opal_Device device, Opal_CommandAllocator command_allocator, Opal_CommandBuffer *command_buffer);
typedef Opal_Result (*PFN_opalCreateShader)(Opal_Device device, const Opal_ShaderDesc *desc, Opal_Shader *shader);
typedef Opal_Result (*PFN_opalCreateDescriptorHeap)(Opal_Device device, const Opal_DescriptorHeapDesc *desc, Opal_DescriptorHeap *descriptor_buffer);
typedef Opal_Result (*PFN_opalCreateDescriptorSetLayout)(Opal_Device device, uint32_t num_entries, const Opal_DescriptorSetLayoutEntry *entries, Opal_DescriptorSetLayout *descriptor_set_layout);
typedef Opal_Result (*PFN_opalCreatePipelineLayout)(Opal_Device device, uint32_t num_descriptor_setlayouts, const Opal_DescriptorSetLayout *descriptor_set_layouts, Opal_PipelineLayout *pipeline_layout);
typedef Opal_Result (*PFN_opalCreateGraphicsPipeline)(Opal_Device device, const Opal_GraphicsPipelineDesc *desc, Opal_GraphicsPipeline *pipeline);
typedef Opal_Result (*PFN_opalCreateMeshletPipeline)(Opal_Device device, const Opal_MeshletPipelineDesc *desc, Opal_GraphicsPipeline *pipeline);
typedef Opal_Result (*PFN_opalCreateComputePipeline)(Opal_Device device, const Opal_ComputePipelineDesc *desc, Opal_ComputePipeline *pipeline);
typedef Opal_Result (*PFN_opalCreateRaytracePipeline)(Opal_Device device, const Opal_RaytracePipelineDesc *desc, Opal_RaytracePipeline *pipeline);
typedef Opal_Result (*PFN_opalCreateSwapchain)(Opal_Device device, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain);

typedef Opal_Result (*PFN_opalDestroySemaphore)(Opal_Device device, Opal_Semaphore semaphore);
typedef Opal_Result (*PFN_opalDestroyBuffer)(Opal_Device device, Opal_Buffer buffer);
typedef Opal_Result (*PFN_opalDestroyTexture)(Opal_Device device, Opal_Texture texture);
typedef Opal_Result (*PFN_opalDestroyTextureView)(Opal_Device device, Opal_TextureView texture_view);
typedef Opal_Result (*PFN_opalDestroySampler)(Opal_Device device, Opal_Sampler sampler);
typedef Opal_Result (*PFN_opalDestroyAccelerationStructure)(Opal_Device device, Opal_AccelerationStructure acceleration_structure);
typedef Opal_Result (*PFN_opalDestroyShaderBindingTable)(Opal_Device device, Opal_ShaderBindingTable shader_binding_table);
typedef Opal_Result (*PFN_opalDestroyCommandAllocator)(Opal_Device device, Opal_CommandAllocator command_allocator);
typedef Opal_Result (*PFN_opalDestroyCommandBuffer)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalDestroyShader)(Opal_Device device, Opal_Shader shader);
typedef Opal_Result (*PFN_opalDestroyDescriptorHeap)(Opal_Device device, Opal_DescriptorHeap descriptor_buffer);
typedef Opal_Result (*PFN_opalDestroyDescriptorSetLayout)(Opal_Device device, Opal_DescriptorSetLayout descriptor_set_layout);
typedef Opal_Result (*PFN_opalDestroyPipelineLayout)(Opal_Device device, Opal_PipelineLayout pipeline_layout);
typedef Opal_Result (*PFN_opalDestroyGraphicsPipeline)(Opal_Device device, Opal_GraphicsPipeline pipeline);
typedef Opal_Result (*PFN_opalDestroyComputePipeline)(Opal_Device device, Opal_ComputePipeline pipeline);
typedef Opal_Result (*PFN_opalDestroyRaytracePipeline)(Opal_Device device, Opal_RaytracePipeline pipeline);
typedef Opal_Result (*PFN_opalDestroySwapchain)(Opal_Device device, Opal_Swapchain swapchain);
typedef Opal_Result (*PFN_opalDestroyDevice)(Opal_Device device);

typedef Opal_Result (*PFN_opalBuildShaderBindingTable)(Opal_Device device, Opal_ShaderBindingTable shader_binding_table, const Opal_ShaderBindingTableBuildDesc *desc);
typedef Opal_Result (*PFN_opalBuildAccelerationStructureInstanceBuffer)(Opal_Device device, const Opal_AccelerationStructureInstanceBufferBuildDesc *desc);
typedef Opal_Result (*PFN_opalResetCommandAllocator)(Opal_Device device, Opal_CommandAllocator command_allocator);
typedef Opal_Result (*PFN_opalAllocateDescriptorSet)(Opal_Device device, const Opal_DescriptorSetAllocationDesc *desc, Opal_DescriptorSet *descriptor_set);
typedef Opal_Result (*PFN_opalFreeDescriptorSet)(Opal_Device device, Opal_DescriptorSet descriptor_set);
typedef Opal_Result (*PFN_opalMapBuffer)(Opal_Device device, Opal_Buffer buffer, void **ptr);
typedef Opal_Result (*PFN_opalUnmapBuffer)(Opal_Device device, Opal_Buffer buffer);
typedef Opal_Result (*PFN_opalWriteBuffer)(Opal_Device device, Opal_Buffer buffer, uint64_t offset, const void *data, uint64_t size);
typedef Opal_Result (*PFN_opalUpdateDescriptorSet)(Opal_Device device, Opal_DescriptorSet descriptor_set, uint32_t num_entries, const Opal_DescriptorSetEntry *entries);
typedef Opal_Result (*PFN_opalBeginCommandBuffer)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalEndCommandBuffer)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalQuerySemaphore)(Opal_Device device, Opal_Semaphore semaphore, uint64_t *value);
typedef Opal_Result (*PFN_opalSignalSemaphore)(Opal_Device device, Opal_Semaphore semaphore, uint64_t value);
typedef Opal_Result (*PFN_opalWaitSemaphore)(Opal_Device device, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds);
typedef Opal_Result (*PFN_opalWaitQueue)(Opal_Device device, Opal_Queue queue);
typedef Opal_Result (*PFN_opalWaitIdle)(Opal_Device device);
typedef Opal_Result (*PFN_opalSubmit)(Opal_Device device, Opal_Queue queue, const Opal_SubmitDesc *desc);
typedef Opal_Result (*PFN_opalAcquire)(Opal_Device device, Opal_Swapchain swapchain, Opal_TextureView *texture_view);
typedef Opal_Result (*PFN_opalPresent)(Opal_Device device, Opal_Swapchain swapchain);

typedef Opal_Result (*PFN_opalCmdSetDescriptorHeap)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_DescriptorHeap descriptor_heap);

typedef Opal_Result (*PFN_opalCmdBeginGraphicsPass)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depth_stencil_attachment);
typedef Opal_Result (*PFN_opalCmdGraphicsSetPipelineLayout)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout);
typedef Opal_Result (*PFN_opalCmdGraphicsSetPipeline)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline);
typedef Opal_Result (*PFN_opalCmdGraphicsSetDescriptorSet)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets);
typedef Opal_Result (*PFN_opalCmdGraphicsSetVertexBuffers)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_VertexBufferView *vertex_buffers);
typedef Opal_Result (*PFN_opalCmdGraphicsSetIndexBuffer)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_IndexBufferView index_buffer);
typedef Opal_Result (*PFN_opalCmdGraphicsSetViewport)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_Viewport viewport);
typedef Opal_Result (*PFN_opalCmdGraphicsSetScissor)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef Opal_Result (*PFN_opalCmdGraphicsDraw)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_vertices, uint32_t num_instances, uint32_t base_vertex, uint32_t base_instance);
typedef Opal_Result (*PFN_opalCmdGraphicsDrawIndexed)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t num_instances, uint32_t base_index, int32_t vertex_offset, uint32_t base_instance);
typedef Opal_Result (*PFN_opalCmdGraphicsMeshletDispatch)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_threadgroups_x, uint32_t num_threadgroups_y, uint32_t num_threadgroups_z);
typedef Opal_Result (*PFN_opalCmdEndGraphicsPass)(Opal_Device device, Opal_CommandBuffer command_buffer);

typedef Opal_Result (*PFN_opalCmdBeginComputePass)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalCmdComputeSetPipelineLayout)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout);
typedef Opal_Result (*PFN_opalCmdComputeSetPipeline)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ComputePipeline pipeline);
typedef Opal_Result (*PFN_opalCmdComputeSetDescriptorSet)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets);
typedef Opal_Result (*PFN_opalCmdComputeDispatch)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_threadgroups_x, uint32_t num_threadgroups_y, uint32_t num_threadgroups_z);
typedef Opal_Result (*PFN_opalCmdEndComputePass)(Opal_Device device, Opal_CommandBuffer command_buffer);

typedef Opal_Result (*PFN_opalCmdBeginRaytracePass)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalCmdRaytraceSetPipelineLayout)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout);
typedef Opal_Result (*PFN_opalCmdRaytraceSetPipeline)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ComputePipeline pipeline);
typedef Opal_Result (*PFN_opalCmdRaytraceSetDescriptorSet)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets);
typedef Opal_Result (*PFN_opalCmdRaytraceSetShaderBindingTable)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTable shader_binding_table);
typedef Opal_Result (*PFN_opalCmdRaytraceDispatch)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t width, uint32_t height, uint32_t depth);
typedef Opal_Result (*PFN_opalCmdEndRaytracePass)(Opal_Device device, Opal_CommandBuffer command_buffer);

typedef Opal_Result (*PFN_opalCmdBeginCopyPass)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalCmdCopyBufferToBuffer)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_Buffer src_buffer, uint64_t src_offset, Opal_Buffer dst_buffer, uint64_t dst_offset, uint64_t size);
typedef Opal_Result (*PFN_opalCmdCopyBufferToTexture)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferTextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size);
typedef Opal_Result (*PFN_opalCmdCopyTextureToBuffer)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferTextureRegion dst, Opal_Extent3D size);
typedef Opal_Result (*PFN_opalCmdCopyTextureToTexture)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size);
typedef Opal_Result (*PFN_opalCmdEndCopyPass)(Opal_Device device, Opal_CommandBuffer command_buffer);

typedef Opal_Result (*PFN_opalCmdBeginAccelerationStructurePass)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalCmdAccelerationStructureBuild)(Opal_Device device, Opal_CommandBuffer command_buffer, const Opal_AccelerationStructureBuildDesc *desc);
typedef Opal_Result (*PFN_opalCmdAccelerationStructureCopy)(Opal_Device device, Opal_CommandBuffer command_buffer, const Opal_AccelerationStructureCopyDesc *desc);
typedef Opal_Result (*PFN_opalCmdEndAccelerationStructurePass)(Opal_Device device, Opal_CommandBuffer command_buffer);

typedef Opal_Result (*PFN_opalCmdBufferTransitionBarrier)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after);
typedef Opal_Result (*PFN_opalCmdTextureTransitionBarrier)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after);

typedef struct Opal_InstanceTable_t
{
	PFN_opalEnumerateDevices enumerateDevices;

	PFN_opalCreateSurface createSurface;
	PFN_opalCreateDevice createDevice;
	PFN_opalCreateDefaultDevice createDefaultDevice;

	PFN_opalDestroySurface destroySurface;
	PFN_opalDestroyInstance destroyInstance;
} Opal_InstanceTable;

typedef struct Opal_DeviceTable_t
{
	PFN_opalGetDeviceInfo getDeviceInfo;
	PFN_opalGetDeviceQueue getDeviceQueue;
	PFN_opalGetAccelerationStructurePrebuildInfo getAccelerationStructurePrebuildInfo;
	PFN_opalGetSupportedSurfaceFormats getSupportedSurfaceFormats;
	PFN_opalGetSupportedPresentModes getSupportedPresentModes;
	PFN_opalGetPreferredSurfaceFormat getPreferredSurfaceFormat;
	PFN_opalGetPreferredSurfacePresentMode getPreferredSurfacePresentMode;

	PFN_opalCreateSemaphore createSemaphore;
	PFN_opalCreateBuffer createBuffer;
	PFN_opalCreateTexture createTexture;
	PFN_opalCreateTextureView createTextureView;
	PFN_opalCreateSampler createSampler;
	PFN_opalCreateAccelerationStructure createAccelerationStructure;
	PFN_opalCreateShaderBindingTable createShaderBindingTable;
	PFN_opalCreateCommandAllocator createCommandAllocator;
	PFN_opalCreateCommandBuffer createCommandBuffer;
	PFN_opalCreateShader createShader;
	PFN_opalCreateDescriptorHeap createDescriptorHeap;
	PFN_opalCreateDescriptorSetLayout createDescriptorSetLayout;
	PFN_opalCreatePipelineLayout createPipelineLayout;
	PFN_opalCreateGraphicsPipeline createGraphicsPipeline;
	PFN_opalCreateMeshletPipeline createMeshletPipeline;
	PFN_opalCreateComputePipeline createComputePipeline;
	PFN_opalCreateRaytracePipeline createRaytracePipeline;
	PFN_opalCreateSwapchain createSwapchain;

	PFN_opalDestroySemaphore destroySemaphore;
	PFN_opalDestroyBuffer destroyBuffer;
	PFN_opalDestroyTexture destroyTexture;
	PFN_opalDestroyTextureView destroyTextureView;
	PFN_opalDestroySampler destroySampler;
	PFN_opalDestroyAccelerationStructure destroyAccelerationStructure;
	PFN_opalDestroyShaderBindingTable destroyShaderBindingTable;
	PFN_opalDestroyCommandAllocator destroyCommandAllocator;
	PFN_opalDestroyCommandBuffer destroyCommandBuffer;
	PFN_opalDestroyShader destroyShader;
	PFN_opalDestroyDescriptorHeap destroyDescriptorHeap;
	PFN_opalDestroyDescriptorSetLayout destroyDescriptorSetLayout;
	PFN_opalDestroyPipelineLayout destroyPipelineLayout;
	PFN_opalDestroyGraphicsPipeline destroyGraphicsPipeline;
	PFN_opalDestroyComputePipeline destroyComputePipeline;
	PFN_opalDestroyRaytracePipeline destroyRaytracePipeline;
	PFN_opalDestroySwapchain destroySwapchain;
	PFN_opalDestroyDevice destroyDevice;

	PFN_opalBuildShaderBindingTable buildShaderBindingTable;
	PFN_opalBuildAccelerationStructureInstanceBuffer buildAccelerationStructureInstanceBuffer;
	PFN_opalResetCommandAllocator resetCommandAllocator;
	PFN_opalAllocateDescriptorSet allocateDescriptorSet;
	PFN_opalFreeDescriptorSet freeDescriptorSet;
	PFN_opalMapBuffer mapBuffer;
	PFN_opalUnmapBuffer unmapBuffer;
	PFN_opalWriteBuffer writeBuffer;
	PFN_opalUpdateDescriptorSet updateDescriptorSet;
	PFN_opalBeginCommandBuffer beginCommandBuffer;
	PFN_opalEndCommandBuffer endCommandBuffer;
	PFN_opalQuerySemaphore querySemaphore;
	PFN_opalSignalSemaphore signalSemaphore;
	PFN_opalWaitSemaphore waitSemaphore;
	PFN_opalWaitQueue waitQueue;
	PFN_opalWaitIdle waitIdle;
	PFN_opalSubmit submit;
	PFN_opalAcquire acquire;
	PFN_opalPresent present;

	PFN_opalCmdSetDescriptorHeap cmdSetDescriptorHeap;

	PFN_opalCmdBeginGraphicsPass cmdBeginGraphicsPass;
	PFN_opalCmdGraphicsSetPipelineLayout cmdGraphicsSetPipelineLayout;
	PFN_opalCmdGraphicsSetPipeline cmdGraphicsSetPipeline;
	PFN_opalCmdGraphicsSetDescriptorSet cmdGraphicsSetDescriptorSet;
	PFN_opalCmdGraphicsSetVertexBuffers cmdGraphicsSetVertexBuffers;
	PFN_opalCmdGraphicsSetIndexBuffer cmdGraphicsSetIndexBuffer;
	PFN_opalCmdGraphicsSetViewport cmdGraphicsSetViewport;
	PFN_opalCmdGraphicsSetScissor cmdGraphicsSetScissor;
	PFN_opalCmdGraphicsDraw cmdGraphicsDraw;
	PFN_opalCmdGraphicsDrawIndexed cmdGraphicsDrawIndexed;
	PFN_opalCmdGraphicsMeshletDispatch cmdGraphicsMeshletDispatch;
	PFN_opalCmdEndGraphicsPass cmdEndGraphicsPass;

	PFN_opalCmdBeginComputePass cmdBeginComputePass;
	PFN_opalCmdComputeSetPipelineLayout cmdComputeSetPipelineLayout;
	PFN_opalCmdComputeSetPipeline cmdComputeSetPipeline;
	PFN_opalCmdComputeSetDescriptorSet cmdComputeSetDescriptorSet;
	PFN_opalCmdComputeDispatch cmdComputeDispatch;
	PFN_opalCmdEndComputePass cmdEndComputePass;

	PFN_opalCmdBeginRaytracePass cmdBeginRaytracePass;
	PFN_opalCmdRaytraceSetPipelineLayout cmdRaytraceSetPipelineLayout;
	PFN_opalCmdRaytraceSetPipeline cmdRaytraceSetPipeline;
	PFN_opalCmdRaytraceSetDescriptorSet cmdRaytraceSetDescriptorSet;
	PFN_opalCmdRaytraceSetShaderBindingTable cmdRaytraceSetShaderBindingTable;
	PFN_opalCmdRaytraceDispatch cmdRaytraceDispatch;
	PFN_opalCmdEndRaytracePass cmdEndRaytracePass;

	PFN_opalCmdBeginCopyPass cmdBeginCopyPass;
	PFN_opalCmdCopyBufferToBuffer cmdCopyBufferToBuffer;
	PFN_opalCmdCopyBufferToTexture cmdCopyBufferToTexture;
	PFN_opalCmdCopyTextureToBuffer cmdCopyTextureToBuffer;
	PFN_opalCmdCopyTextureToTexture cmdCopyTextureToTexture;
	PFN_opalCmdEndCopyPass cmdEndCopyPass;

	PFN_opalCmdBeginAccelerationStructurePass cmdBeginAccelerationStructurePass;
	PFN_opalCmdAccelerationStructureBuild cmdAccelerationStructureBuild;
	PFN_opalCmdAccelerationStructureCopy cmdAccelerationStructureCopy;
	PFN_opalCmdEndAccelerationStructurePass cmdEndAccelerationStructurePass;

	PFN_opalCmdBufferTransitionBarrier cmdBufferTransitionBarrier;
	PFN_opalCmdTextureTransitionBarrier cmdTextureTransitionBarrier;
} Opal_DeviceTable;

// API
#if !defined(OPAL_NO_PROTOTYPES)
OPAL_APIENTRY Opal_Result opalCreateInstance(Opal_Api api, const Opal_InstanceDesc *desc, Opal_Instance *instance);
OPAL_APIENTRY Opal_Result opalGetInstanceTable(Opal_Instance instance, Opal_InstanceTable *instance_table);
OPAL_APIENTRY Opal_Result opalGetDeviceTable(Opal_Device device, Opal_DeviceTable *device_table);

OPAL_APIENTRY Opal_Result opalEnumerateDevices(Opal_Instance instance, uint32_t *device_count, Opal_DeviceInfo *infos);

OPAL_APIENTRY Opal_Result opalCreateSurface(Opal_Instance instance, void *handle, Opal_Surface *surface);
OPAL_APIENTRY Opal_Result opalCreateDevice(Opal_Instance instance, uint32_t index, Opal_Device *device);
OPAL_APIENTRY Opal_Result opalCreateDefaultDevice(Opal_Instance instance, Opal_DeviceHint hint, Opal_Device *device);

OPAL_APIENTRY Opal_Result opalDestroySurface(Opal_Instance instance, Opal_Surface surface);
OPAL_APIENTRY Opal_Result opalDestroyInstance(Opal_Instance instance);

OPAL_APIENTRY Opal_Result opalGetDeviceInfo(Opal_Device device, Opal_DeviceInfo *info);
OPAL_APIENTRY Opal_Result opalGetDeviceQueue(Opal_Device device, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue);
OPAL_APIENTRY Opal_Result opalGetAccelerationStructurePrebuildInfo(Opal_Device device, const Opal_AccelerationStructureBuildDesc *desc, Opal_AccelerationStructurePrebuildInfo *info);
OPAL_APIENTRY Opal_Result opalGetSupportedSurfaceFormats(Opal_Device device, Opal_Surface surface, uint32_t *num_formats, Opal_SurfaceFormat *formats);
OPAL_APIENTRY Opal_Result opalGetSupportedPresentModes(Opal_Device device, Opal_Surface surface, uint32_t *num_present_modes, Opal_PresentMode *present_modes);
OPAL_APIENTRY Opal_Result opalGetPreferredSurfaceFormat(Opal_Device device, Opal_Surface surface, Opal_SurfaceFormat *format);
OPAL_APIENTRY Opal_Result opalGetPreferredSurfacePresentMode(Opal_Device device, Opal_Surface surface, Opal_PresentMode *present_mode);

OPAL_APIENTRY Opal_Result opalCreateSemaphore(Opal_Device device, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore);
OPAL_APIENTRY Opal_Result opalCreateBuffer(Opal_Device device, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
OPAL_APIENTRY Opal_Result opalCreateTexture(Opal_Device device, const Opal_TextureDesc *desc, Opal_Texture *texture);
OPAL_APIENTRY Opal_Result opalCreateTextureView(Opal_Device device, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);
OPAL_APIENTRY Opal_Result opalCreateSampler(Opal_Device device, const Opal_SamplerDesc *desc, Opal_Sampler *sampler);
OPAL_APIENTRY Opal_Result opalCreateAccelerationStructure(Opal_Device device, const Opal_AccelerationStructureDesc *desc, Opal_AccelerationStructure *acceleration_structure);
OPAL_APIENTRY Opal_Result opalCreateShaderBindingTable(Opal_Device device, Opal_RaytracePipeline pipeline, Opal_ShaderBindingTable *shader_binding_table);
OPAL_APIENTRY Opal_Result opalCreateCommandAllocator(Opal_Device device, Opal_Queue queue, Opal_CommandAllocator *command_allocator);
OPAL_APIENTRY Opal_Result opalCreateCommandBuffer(Opal_Device device, Opal_CommandAllocator command_allocator, Opal_CommandBuffer *command_buffer);
OPAL_APIENTRY Opal_Result opalCreateShader(Opal_Device device, const Opal_ShaderDesc *desc, Opal_Shader *shader);
OPAL_APIENTRY Opal_Result opalCreateDescriptorHeap(Opal_Device device, const Opal_DescriptorHeapDesc *desc, Opal_DescriptorHeap *descriptor_buffer);
OPAL_APIENTRY Opal_Result opalCreateDescriptorSetLayout(Opal_Device device, uint32_t num_entries, const Opal_DescriptorSetLayoutEntry *entries, Opal_DescriptorSetLayout *descriptor_set_layout);
OPAL_APIENTRY Opal_Result opalCreatePipelineLayout(Opal_Device device, uint32_t num_descriptor_setlayouts, const Opal_DescriptorSetLayout *descriptor_set_layouts, Opal_PipelineLayout *pipeline_layout);
OPAL_APIENTRY Opal_Result opalCreateGraphicsPipeline(Opal_Device device, const Opal_GraphicsPipelineDesc *desc, Opal_GraphicsPipeline *pipeline);
OPAL_APIENTRY Opal_Result opalCreateMeshletPipeline(Opal_Device device, const Opal_MeshletPipelineDesc *desc, Opal_GraphicsPipeline *pipeline);
OPAL_APIENTRY Opal_Result opalCreateComputePipeline(Opal_Device device, const Opal_ComputePipelineDesc *desc, Opal_ComputePipeline *pipeline);
OPAL_APIENTRY Opal_Result opalCreateRaytracePipeline(Opal_Device device, const Opal_RaytracePipelineDesc *desc, Opal_RaytracePipeline *pipeline);
OPAL_APIENTRY Opal_Result opalCreateSwapchain(Opal_Device device, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain);

OPAL_APIENTRY Opal_Result opalDestroySemaphore(Opal_Device device, Opal_Semaphore semaphore);
OPAL_APIENTRY Opal_Result opalDestroyBuffer(Opal_Device device, Opal_Buffer buffer);
OPAL_APIENTRY Opal_Result opalDestroyTexture(Opal_Device device, Opal_Texture texture);
OPAL_APIENTRY Opal_Result opalDestroyTextureView(Opal_Device device, Opal_TextureView texture_view);
OPAL_APIENTRY Opal_Result opalDestroySampler(Opal_Device device, Opal_Sampler sampler);
OPAL_APIENTRY Opal_Result opalDestroyAccelerationStructure(Opal_Device device, Opal_AccelerationStructure acceleration_structure);
OPAL_APIENTRY Opal_Result opalDestroyShaderBindingTable(Opal_Device device, Opal_ShaderBindingTable shader_binding_table);
OPAL_APIENTRY Opal_Result opalDestroyCommandAllocator(Opal_Device device, Opal_CommandAllocator command_allocator);
OPAL_APIENTRY Opal_Result opalDestroyCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalDestroyShader(Opal_Device device, Opal_Shader shader);
OPAL_APIENTRY Opal_Result opalDestroyDescriptorHeap(Opal_Device device, Opal_DescriptorHeap descriptor_buffer);
OPAL_APIENTRY Opal_Result opalDestroyDescriptorSetLayout(Opal_Device device, Opal_DescriptorSetLayout descriptor_set_layout);
OPAL_APIENTRY Opal_Result opalDestroyPipelineLayout(Opal_Device device, Opal_PipelineLayout pipeline_layout);
OPAL_APIENTRY Opal_Result opalDestroyGraphicsPipeline(Opal_Device device, Opal_GraphicsPipeline pipeline);
OPAL_APIENTRY Opal_Result opalDestroyComputePipeline(Opal_Device device, Opal_ComputePipeline pipeline);
OPAL_APIENTRY Opal_Result opalDestroyRaytracePipeline(Opal_Device device, Opal_RaytracePipeline pipeline);
OPAL_APIENTRY Opal_Result opalDestroySwapchain(Opal_Device device, Opal_Swapchain swapchain);
OPAL_APIENTRY Opal_Result opalDestroyDevice(Opal_Device device);

OPAL_APIENTRY Opal_Result opalBuildShaderBindingTable(Opal_Device device, Opal_ShaderBindingTable shader_binding_table, const Opal_ShaderBindingTableBuildDesc *desc);
OPAL_APIENTRY Opal_Result opalBuildAccelerationStructureInstanceBuffer(Opal_Device device, const Opal_AccelerationStructureInstanceBufferBuildDesc *desc);
OPAL_APIENTRY Opal_Result opalResetCommandAllocator(Opal_Device device, Opal_CommandAllocator command_allocator);
OPAL_APIENTRY Opal_Result opalAllocateDescriptorSet(Opal_Device device, const Opal_DescriptorSetAllocationDesc *desc, Opal_DescriptorSet *descriptor_set);
OPAL_APIENTRY Opal_Result opalFreeDescriptorSet(Opal_Device device, Opal_DescriptorSet descriptor_set);
OPAL_APIENTRY Opal_Result opalMapBuffer(Opal_Device device, Opal_Buffer buffer, void **ptr);
OPAL_APIENTRY Opal_Result opalUnmapBuffer(Opal_Device device, Opal_Buffer buffer);
OPAL_APIENTRY Opal_Result opalWriteBuffer(Opal_Device device, Opal_Buffer buffer, uint64_t offset, const void *data, uint64_t size);
OPAL_APIENTRY Opal_Result opalUpdateDescriptorSet(Opal_Device device, Opal_DescriptorSet descriptor_set, uint32_t num_entries, const Opal_DescriptorSetEntry *entries);
OPAL_APIENTRY Opal_Result opalBeginCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalEndCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalQuerySemaphore(Opal_Device device, Opal_Semaphore semaphore, uint64_t *value);
OPAL_APIENTRY Opal_Result opalSignalSemaphore(Opal_Device device, Opal_Semaphore semaphore, uint64_t value);
OPAL_APIENTRY Opal_Result opalWaitSemaphore(Opal_Device device, Opal_Semaphore semaphore, uint64_t value, uint64_t timeout_milliseconds);
OPAL_APIENTRY Opal_Result opalWaitQueue(Opal_Device device, Opal_Queue queue);
OPAL_APIENTRY Opal_Result opalWaitIdle(Opal_Device device);
OPAL_APIENTRY Opal_Result opalSubmit(Opal_Device device, Opal_Queue queue, const Opal_SubmitDesc *desc);
OPAL_APIENTRY Opal_Result opalAcquire(Opal_Device device, Opal_Swapchain swapchain, Opal_TextureView *texture_view);
OPAL_APIENTRY Opal_Result opalPresent(Opal_Device device, Opal_Swapchain swapchain);

OPAL_APIENTRY Opal_Result opalCmdSetDescriptorHeap(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_DescriptorHeap descriptor_heap);

OPAL_APIENTRY Opal_Result opalCmdBeginGraphicsPass(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depth_stencil_attachment);
OPAL_APIENTRY Opal_Result opalCmdGraphicsSetPipelineLayout(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout);
OPAL_APIENTRY Opal_Result opalCmdGraphicsSetPipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_GraphicsPipeline pipeline);
OPAL_APIENTRY Opal_Result opalCmdGraphicsSetDescriptorSet(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets);
OPAL_APIENTRY Opal_Result opalCmdGraphicsSetVertexBuffers(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_VertexBufferView *vertex_buffers);
OPAL_APIENTRY Opal_Result opalCmdGraphicsSetIndexBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_IndexBufferView index_buffer);
OPAL_APIENTRY Opal_Result opalCmdGraphicsSetViewport(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_Viewport viewport);
OPAL_APIENTRY Opal_Result opalCmdGraphicsSetScissor(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
OPAL_APIENTRY Opal_Result opalCmdGraphicsDraw(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_vertices, uint32_t num_instances, uint32_t base_vertex, uint32_t base_instance);
OPAL_APIENTRY Opal_Result opalCmdGraphicsDrawIndexed(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t num_instances, uint32_t base_index, int32_t vertex_offset, uint32_t base_instance);
OPAL_APIENTRY Opal_Result opalCmdGraphicsMeshletDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_threadgroups_x, uint32_t num_threadgroups_y, uint32_t num_threadgroups_z);
OPAL_APIENTRY Opal_Result opalCmdEndGraphicsPass(Opal_Device device, Opal_CommandBuffer command_buffer);

OPAL_APIENTRY Opal_Result opalCmdBeginComputePass(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalCmdComputeSetPipelineLayout(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout);
OPAL_APIENTRY Opal_Result opalCmdComputeSetPipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ComputePipeline pipeline);
OPAL_APIENTRY Opal_Result opalCmdComputeSetDescriptorSet(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets);
OPAL_APIENTRY Opal_Result opalCmdComputeDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_threadgroups_x, uint32_t num_threadgroups_y, uint32_t num_threadgroups_z);
OPAL_APIENTRY Opal_Result opalCmdEndComputePass(Opal_Device device, Opal_CommandBuffer command_buffer);

OPAL_APIENTRY Opal_Result opalCmdBeginRaytracePass(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalCmdRaytraceSetPipelineLayout(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout);
OPAL_APIENTRY Opal_Result opalCmdRaytraceSetPipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ComputePipeline pipeline);
OPAL_APIENTRY Opal_Result opalCmdRaytraceSetDescriptorSet(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t index, Opal_DescriptorSet descriptor_set, uint32_t num_dynamic_offsets, const uint32_t *dynamic_offsets);
OPAL_APIENTRY Opal_Result opalCmdRaytraceSetShaderBindingTable(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTable shader_binding_table);
OPAL_APIENTRY Opal_Result opalCmdRaytraceDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t width, uint32_t height, uint32_t depth);
OPAL_APIENTRY Opal_Result opalCmdEndRaytracePass(Opal_Device device, Opal_CommandBuffer command_buffer);

OPAL_APIENTRY Opal_Result opalCmdBeginCopyPass(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalCmdCopyBufferToBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_Buffer src_buffer, uint64_t src_offset, Opal_Buffer dst_buffer, uint64_t dst_offset, uint64_t size);
OPAL_APIENTRY Opal_Result opalCmdCopyBufferToTexture(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferTextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size);
OPAL_APIENTRY Opal_Result opalCmdCopyTextureToBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferTextureRegion dst, Opal_Extent3D size);
OPAL_APIENTRY Opal_Result opalCmdCopyTextureToTexture(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_TextureRegion dst, Opal_Extent3D size);
OPAL_APIENTRY Opal_Result opalCmdEndCopyPass(Opal_Device device, Opal_CommandBuffer command_buffer);

OPAL_APIENTRY Opal_Result opalCmdBeginAccelerationStructurePass(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalCmdAccelerationStructureBuild(Opal_Device device, Opal_CommandBuffer command_buffer, const Opal_AccelerationStructureBuildDesc *desc);
OPAL_APIENTRY Opal_Result opalCmdAccelerationStructureCopy(Opal_Device device, Opal_CommandBuffer command_buffer, const Opal_AccelerationStructureCopyDesc *desc);
OPAL_APIENTRY Opal_Result opalCmdEndAccelerationStructurePass(Opal_Device device, Opal_CommandBuffer command_buffer);

OPAL_APIENTRY Opal_Result opalCmdBufferTransitionBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after);
OPAL_APIENTRY Opal_Result opalCmdTextureTransitionBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after);
#endif

#ifdef __cplusplus
}
#endif
