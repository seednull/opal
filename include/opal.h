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
OPAL_DEFINE_HANDLE(Opal_CommandPool);
OPAL_DEFINE_HANDLE(Opal_CommandBuffer);
OPAL_DEFINE_HANDLE(Opal_Shader);
OPAL_DEFINE_HANDLE(Opal_BindsetLayout);
OPAL_DEFINE_HANDLE(Opal_BindsetPool);
OPAL_DEFINE_HANDLE(Opal_Bindset);
OPAL_DEFINE_HANDLE(Opal_PipelineLayout);
OPAL_DEFINE_HANDLE(Opal_Pipeline);
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
	OPAL_INVALID_BUFFER,
	OPAL_INVALID_BINDING_INDEX,
	OPAL_NO_MEMORY,
	OPAL_SWAPCHAIN_PRESENT_NOT_SUPPORTED,
	OPAL_SWAPCHAIN_PRESENT_MODE_NOT_SUPPORTED,
	OPAL_SWAPCHAIN_FORMAT_NOT_SUPPORTED,

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

	OPAL_API_DEFAULT = OPAL_API_VULKAN,

	OPAL_API_ENUM_MAX,
	OPAL_API_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_Api;

typedef enum Opal_InstanceCreationFlags_t
{
	OPAL_INSTANCE_CREATION_FLAGS_USE_VMA = 0x00000001,
	OPAL_INSTANCE_CREATION_FLAGS_USE_VULKAN_VALIDATION_LAYERS = 0x00000002,

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

typedef enum Opal_ResourceState_t
{
	OPAL_RESOURCE_STATE_GENERAL = 0x00000000,
	OPAL_RESOURCE_STATE_VERTEX_AND_UNIFORM_BUFFER = 0x00000001,
	OPAL_RESOURCE_STATE_INDEX_BUFFER = 0x00000002,
	OPAL_RESOURCE_STATE_FRAMEBUFFER_ATTACHMENT = 0x00000004,
	OPAL_RESOURCE_STATE_UNORDERED_ACCESS = 0x00000008,
	OPAL_RESOURCE_STATE_DEPTHSTENCIL_WRITE = 0x00000010,
	OPAL_RESOURCE_STATE_DEPTHSTENCIL_READ = 0x00000020,
	OPAL_RESOURCE_STATE_NON_FRAGMENT_SHADER_RESOURCE = 0x00000040,
	OPAL_RESOURCE_STATE_FRAGMENT_SHADER_RESOURCE = 0x00000080,
	OPAL_RESOURCE_STATE_COPY_DEST = 0x00000100,
	OPAL_RESOURCE_STATE_COPY_SOURCE = 0x00000200,
	OPAL_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE = 0x00000400,
	OPAL_RESOURCE_STATE_PRESENT = 0x00000800,

	OPAL_RESOURCE_STATE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_ResourceState;

typedef enum Opal_BufferUsageFlags_t
{
	OPAL_BUFFER_USAGE_TRANSFER_SRC = 0x00000001,
	OPAL_BUFFER_USAGE_TRANSFER_DST = 0x00000002,
	OPAL_BUFFER_USAGE_VERTEX = 0x00000004,
	OPAL_BUFFER_USAGE_INDEX = 0x00000008,
	OPAL_BUFFER_USAGE_UNIFORM = 0x00000010,
	OPAL_BUFFER_USAGE_STORAGE = 0x00000020,
	OPAL_BUFFER_USAGE_INDIRECT = 0x00000040,

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

typedef enum Opal_Colorspace_t
{
	OPAL_COLORSPACE_SRGB = 0,

	OPAL_COLORSPACE_ENUM_MAX,
	OPAL_COLORSPACE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_Colorspace;

typedef enum Opal_Format_t
{
	OPAL_FORMAT_UNDEFINED = 0,

	OPAL_FORMAT_R8_UNORM,
	OPAL_FORMAT_R8_SNORM,
	OPAL_FORMAT_R8_UINT,
	OPAL_FORMAT_R8_SINT,
	OPAL_FORMAT_R8_SRGB,

	OPAL_FORMAT_RG8_UNORM,
	OPAL_FORMAT_RG8_SNORM,
	OPAL_FORMAT_RG8_UINT,
	OPAL_FORMAT_RG8_SINT,
	OPAL_FORMAT_RG8_SRGB,

	OPAL_FORMAT_RGB8_UNORM,
	OPAL_FORMAT_RGB8_SNORM,
	OPAL_FORMAT_RGB8_UINT,
	OPAL_FORMAT_RGB8_SINT,
	OPAL_FORMAT_RGB8_SRGB,

	OPAL_FORMAT_RGBA8_UNORM,
	OPAL_FORMAT_RGBA8_SNORM,
	OPAL_FORMAT_RGBA8_UINT,
	OPAL_FORMAT_RGBA8_SINT,
	OPAL_FORMAT_RGBA8_SRGB,

	OPAL_FORMAT_BGRA8_UNORM,
	OPAL_FORMAT_BGRA8_SNORM,
	OPAL_FORMAT_BGRA8_UINT,
	OPAL_FORMAT_BGRA8_SINT,
	OPAL_FORMAT_BGRA8_SRGB,

	OPAL_FORMAT_R16_UNORM,
	OPAL_FORMAT_R16_SNORM,
	OPAL_FORMAT_R16_UINT,
	OPAL_FORMAT_R16_SINT,
	OPAL_FORMAT_R16_SFLOAT,

	OPAL_FORMAT_RG16_UNORM,
	OPAL_FORMAT_RG16_SNORM,
	OPAL_FORMAT_RG16_UINT,
	OPAL_FORMAT_RG16_SINT,
	OPAL_FORMAT_RG16_SFLOAT,

	OPAL_FORMAT_RGB16_UNORM,
	OPAL_FORMAT_RGB16_SNORM,
	OPAL_FORMAT_RGB16_UINT,
	OPAL_FORMAT_RGB16_SINT,
	OPAL_FORMAT_RGB16_SFLOAT,

	OPAL_FORMAT_RGBA16_UNORM,
	OPAL_FORMAT_RGBA16_SNORM,
	OPAL_FORMAT_RGBA16_UINT,
	OPAL_FORMAT_RGBA16_SINT,
	OPAL_FORMAT_RGBA16_SFLOAT,

	OPAL_FORMAT_R32_UINT,
	OPAL_FORMAT_R32_SINT,
	OPAL_FORMAT_R32_SFLOAT,

	OPAL_FORMAT_RG32_UINT,
	OPAL_FORMAT_RG32_SINT,
	OPAL_FORMAT_RG32_SFLOAT,

	OPAL_FORMAT_RGB32_UINT,
	OPAL_FORMAT_RGB32_SINT,
	OPAL_FORMAT_RGB32_SFLOAT,

	OPAL_FORMAT_RGBA32_UINT,
	OPAL_FORMAT_RGBA32_SINT,
	OPAL_FORMAT_RGBA32_SFLOAT,

	OPAL_FORMAT_B10G11R11_UFLOAT,
	OPAL_FORMAT_E5B9G9R9_UFLOAT,

	OPAL_FORMAT_BC1_RGB_UNORM,
	OPAL_FORMAT_BC1_RGB_SRGB,
	OPAL_FORMAT_BC1_RGBA_UNORM,
	OPAL_FORMAT_BC1_RGBA_SRGB,
	OPAL_FORMAT_BC2_UNORM,
	OPAL_FORMAT_BC2_SRGB,
	OPAL_FORMAT_BC3_UNORM,
	OPAL_FORMAT_BC3_SRGB,
	OPAL_FORMAT_BC4_UNORM,
	OPAL_FORMAT_BC4_SNORM,
	OPAL_FORMAT_BC5_UNORM,
	OPAL_FORMAT_BC5_SNORM,
	OPAL_FORMAT_BC6H_UFLOAT,
	OPAL_FORMAT_BC6H_SFLOAT,
	OPAL_FORMAT_BC7_UNORM,
	OPAL_FORMAT_BC7_SRGB,

	OPAL_FORMAT_ETC2_R8G8B8_UNORM,
	OPAL_FORMAT_ETC2_R8G8B8_SRGB,
	OPAL_FORMAT_ETC2_R8G8B8A1_UNORM,
	OPAL_FORMAT_ETC2_R8G8B8A1_SRGB,
	OPAL_FORMAT_ETC2_R8G8B8A8_UNORM,
	OPAL_FORMAT_ETC2_R8G8B8A8_SRGB,
	OPAL_FORMAT_EAC_R11_UNORM,
	OPAL_FORMAT_EAC_R11_SNORM,
	OPAL_FORMAT_EAC_R11G11_UNORM,
	OPAL_FORMAT_EAC_R11G11_SNORM,

	OPAL_FORMAT_ASTC_4x4_UNORM,
	OPAL_FORMAT_ASTC_4x4_SRGB,
	OPAL_FORMAT_ASTC_5x4_UNORM,
	OPAL_FORMAT_ASTC_5x4_SRGB,
	OPAL_FORMAT_ASTC_5x5_UNORM,
	OPAL_FORMAT_ASTC_5x5_SRGB,
	OPAL_FORMAT_ASTC_6x5_UNORM,
	OPAL_FORMAT_ASTC_6x5_SRGB,
	OPAL_FORMAT_ASTC_6x6_UNORM,
	OPAL_FORMAT_ASTC_6x6_SRGB,
	OPAL_FORMAT_ASTC_8x5_UNORM,
	OPAL_FORMAT_ASTC_8x5_SRGB,
	OPAL_FORMAT_ASTC_8x6_UNORM,
	OPAL_FORMAT_ASTC_8x6_SRGB,
	OPAL_FORMAT_ASTC_8x8_UNORM,
	OPAL_FORMAT_ASTC_8x8_SRGB,
	OPAL_FORMAT_ASTC_10x5_UNORM,
	OPAL_FORMAT_ASTC_10x5_SRGB,
	OPAL_FORMAT_ASTC_10x6_UNORM,
	OPAL_FORMAT_ASTC_10x6_SRGB,
	OPAL_FORMAT_ASTC_10x8_UNORM,
	OPAL_FORMAT_ASTC_10x8_SRGB,
	OPAL_FORMAT_ASTC_10x10_UNORM,
	OPAL_FORMAT_ASTC_10x10_SRGB,
	OPAL_FORMAT_ASTC_12x10_UNORM,
	OPAL_FORMAT_ASTC_12x10_SRGB,
	OPAL_FORMAT_ASTC_12x12_UNORM,
	OPAL_FORMAT_ASTC_12x12_SRGB,

	OPAL_FORMAT_D16_UNORM,
	OPAL_FORMAT_D32_SFLOAT,
	OPAL_FORMAT_D16_UNORM_S8_UINT,
	OPAL_FORMAT_D24_UNORM_S8_UINT,
	OPAL_FORMAT_D32_SFLOAT_S8_UINT,

	OPAL_FORMAT_COLOR_BEGIN = OPAL_FORMAT_R8_UNORM,
	OPAL_FORMAT_COLOR_END = OPAL_FORMAT_ASTC_12x12_SRGB,

	OPAL_FORMAT_DEPTHSTENCIL_BEGIN = OPAL_FORMAT_D16_UNORM,
	OPAL_FORMAT_DEPTHSTENCIL_END = OPAL_FORMAT_D32_SFLOAT_S8_UINT,

	OPAL_FORMAT_ENUM_MAX,
	OPAL_FORMAT_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_Format;

typedef enum Opal_IndexFormat_t
{
	OPAL_INDEX_FORMAT_UINT16 = 0,
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
	OPAL_SHADER_SOURCE_TYPE_WGSL,
	OPAL_SHADER_SOURCE_TYPE_MSL,

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

typedef enum Opal_BindingType_t
{
	OPAL_BINDING_TYPE_UNIFORM_BUFFER = 0,
	OPAL_BINDING_TYPE_COMBINED_TEXTURE_SAMPLER,
	// TODO: more types

	OPAL_BINDING_TYPE_ENUM_MAX,
	OPAL_BINDING_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_BindingType;

typedef enum Opal_PrimitiveType_t {
	OPAL_PRIMITIVE_TYPE_POINT = 0,
	OPAL_PRIMITIVE_TYPE_LINE,
	OPAL_PRIMITIVE_TYPE_TRIANGLE,
	OPAL_PRIMITIVE_TYPE_TRIANGLE_PATCH,
	OPAL_PRIMITIVE_TYPE_QUAD_PATCH,

	OPAL_PRIMITIVE_TYPE_ENUM_MAX,
	OPAL_PRIMITIVE_TYPE_ENUM_FORCE32 = 0x7FFFFFFF,
} Opal_PrimitiveType;

typedef enum Opal_CullMode_t {
	OPAL_CULL_MODE_NONE = 0,
	OPAL_CULL_MODE_FRONT,
	OPAL_CULL_MODE_BACK,
	OPAL_CULL_MODE_FRONT_AND_BACK,

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
typedef struct Opal_DeviceInfo_t
{
	char name[256];
	Opal_DeviceType device_type;
	uint64_t driver_version;
	uint32_t vendor_id;
	uint32_t device_id;
	uint8_t tessellation_shader : 1;
	uint8_t geometry_shader : 1;
	uint8_t compute_pipeline : 1;
	uint8_t meshlet_pipeline : 1;
	uint8_t raytrace_pipeline : 1;
	uint8_t texture_compression_etc2 : 1;
	uint8_t texture_compression_astc : 1;
	uint8_t texture_compression_bc : 1;
	uint64_t max_buffer_alignment;
	uint32_t queue_count[OPAL_DEVICE_ENGINE_TYPE_ENUM_MAX];
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
	Opal_Format format;
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
	Opal_CompareOp compare_op;
} Opal_SamplerDesc;

typedef struct Opal_ShaderDesc_t
{
	Opal_ShaderSourceType type;
	const void *data;
	uint64_t size;
} Opal_ShaderDesc;

typedef union Opal_ClearValue_t
{
	union
	{
		float f[4];
		uint32_t u[4];
		int32_t i[4];
	} color;
	struct
	{
		float depth;
		uint32_t stencil;
	};
} Opal_ClearValue;

typedef struct Opal_FramebufferAttachment_t
{
	Opal_TextureView texture_view;
	Opal_TextureView resolve_texture_view;
	Opal_LoadOp load_op;
	Opal_StoreOp store_op;
	Opal_ClearValue clear_value;
} Opal_FramebufferAttachment;

typedef struct Opal_TextureRegion_t
{
	Opal_TextureView texture_view;
	int32_t offset_x;
	int32_t offset_y;
	int32_t offset_z;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
} Opal_TextureRegion;

typedef struct Opal_BufferView_t
{
	Opal_Buffer buffer;
	uint64_t offset;
	uint64_t size;
} Opal_BufferView;

typedef struct Opal_ShaderBindingTableView_t
{
	// TODO: handle?
	uint32_t base_raygen_index;
	uint32_t base_hitgroup_index;
	uint32_t base_miss_index;
} Opal_ShaderBindingTableView;

typedef struct Opal_VertexAttribute_t
{
	Opal_Format format;
	uint32_t offset;
} Opal_VertexAttribute;

typedef struct Opal_VertexStream_t
{
	uint32_t stride;
	uint32_t num_vertex_attributes;
	const Opal_VertexAttribute *attributes;
	Opal_VertexInputRate rate;
} Opal_VertexStream;

typedef struct Opal_BindsetLayoutBinding_t
{
	uint32_t binding;
	Opal_BindingType type;
	Opal_ShaderStage visibility;
} Opal_BindsetLayoutBinding;

typedef struct Opal_BindsetBinding_t
{
	uint32_t binding;
	union
	{
		Opal_BufferView buffer;
		struct
		{
			Opal_TextureView texture_view;
			Opal_Sampler sampler;
		};
		// TODO: add more types
	};
} Opal_BindsetBinding;

typedef struct Opal_Viewport_t {
	float x;
	float y;
	float width;
	float height;
	float min_depth;
	float max_depth;
} Opal_Viewport;

typedef struct Opal_BlendState_t {
	uint32_t enable;
	Opal_BlendFactor src_color;
	Opal_BlendFactor dst_color;
	Opal_BlendOp color_op;
	Opal_BlendFactor src_alpha;
	Opal_BlendFactor dst_alpha;
	Opal_BlendOp alpha_op;
} Opal_BlendState;

typedef struct Opal_StencilFaceState_t {
	Opal_StencilOp pass_op;
	Opal_StencilOp depth_fail_op;
	Opal_StencilOp fail_op;
	Opal_CompareOp compare_op;
} Opal_StencilFaceState;

typedef struct Opal_GraphicsPipelineDesc_t
{
	Opal_PipelineLayout pipeline_layout;

	Opal_Shader vertex_shader;
	Opal_Shader tessellation_control_shader;
	Opal_Shader tessellation_evaluation_shader;
	Opal_Shader geometry_shader;
	Opal_Shader fragment_shader;

	uint32_t num_vertex_streams;
	const Opal_VertexStream *vertex_streams;
	Opal_PrimitiveType primitive_type;

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
	Opal_Format color_attachment_formats[8];
	Opal_BlendState color_blend_states[8];

	Opal_Format *depthstencil_attachment_format;
} Opal_GraphicsPipelineDesc;

typedef struct Opal_MeshletPipelineDesc_t
{
	Opal_PipelineLayout pipeline_layout;

	Opal_Shader task_shader;
	Opal_Shader mesh_shader;
	Opal_Shader fragment_shader;

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
	Opal_Format color_attachment_formats[8];
	Opal_BlendState color_blend_states[8];

	Opal_Format *depthstencil_attachment_format;
} Opal_MeshletPipelineDesc;

typedef struct Opal_ComputePipelineDesc_t
{
	Opal_PipelineLayout pipeline_layout;
	Opal_Shader compute_shader;
} Opal_ComputePipelineDesc;

typedef struct Opal_RaytracePipelineDesc_t
{
	Opal_PipelineLayout pipeline_layout;

	uint32_t num_raygen_shaders;
	const Opal_Shader *raygen_shaders;

	uint32_t num_hitgroup_shaders;
	const Opal_Shader *intersection_shaders;
	const Opal_Shader *anyhit_shaders;
	const Opal_Shader *closesthit_shaders;

	uint32_t num_miss_shaders;
	const Opal_Shader *miss_shaders;

	// TODO: additional state (payload size, recursion depth, etc.)
} Opal_RaytracePipelineDesc;

typedef struct Opal_SwapchainDesc_t
{
	Opal_PresentMode mode;
	Opal_Format format;
	Opal_Colorspace colorspace;
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

typedef Opal_Result (*PFN_opalCreateSemaphore)(Opal_Device device, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore);
typedef Opal_Result (*PFN_opalCreateBuffer)(Opal_Device device, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
typedef Opal_Result (*PFN_opalCreateTexture)(Opal_Device device, const Opal_TextureDesc *desc, Opal_Texture *texture);
typedef Opal_Result (*PFN_opalCreateTextureView)(Opal_Device device, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);
typedef Opal_Result (*PFN_opalCreateSampler)(Opal_Device device, const Opal_SamplerDesc *desc, Opal_Sampler *sampler);
typedef Opal_Result (*PFN_opalCreateCommandPool)(Opal_Device device, Opal_Queue queue, Opal_CommandPool *command_pool);
typedef Opal_Result (*PFN_opalCreateShader)(Opal_Device device, const Opal_ShaderDesc *desc, Opal_Shader *shader);
typedef Opal_Result (*PFN_opalCreateBindsetLayout)(Opal_Device device, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout);
typedef Opal_Result (*PFN_opalCreateBindsetPool)(Opal_Device device, Opal_BindsetLayout bindset_layout, uint32_t max_bindsets, Opal_BindsetPool *bindset_pool);
typedef Opal_Result (*PFN_opalCreatePipelineLayout)(Opal_Device device, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout);
typedef Opal_Result (*PFN_opalCreateGraphicsPipeline)(Opal_Device device, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline);
typedef Opal_Result (*PFN_opalCreateMeshletPipeline)(Opal_Device device, const Opal_MeshletPipelineDesc *desc, Opal_Pipeline *pipeline);
typedef Opal_Result (*PFN_opalCreateComputePipeline)(Opal_Device device, const Opal_ComputePipelineDesc *desc, Opal_Pipeline *pipeline);
typedef Opal_Result (*PFN_opalCreateRaytracePipeline)(Opal_Device device, const Opal_RaytracePipelineDesc *desc, Opal_Pipeline *pipeline);
typedef Opal_Result (*PFN_opalCreateSwapchain)(Opal_Device device, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain);

typedef Opal_Result (*PFN_opalDestroySemaphore)(Opal_Device device, Opal_Semaphore semaphore);
typedef Opal_Result (*PFN_opalDestroyBuffer)(Opal_Device device, Opal_Buffer buffer);
typedef Opal_Result (*PFN_opalDestroyTexture)(Opal_Device device, Opal_Texture texture);
typedef Opal_Result (*PFN_opalDestroyTextureView)(Opal_Device device, Opal_TextureView texture_view);
typedef Opal_Result (*PFN_opalDestroySampler)(Opal_Device device, Opal_Sampler sampler);
typedef Opal_Result (*PFN_opalDestroyCommandPool)(Opal_Device device, Opal_CommandPool command_pool);
typedef Opal_Result (*PFN_opalDestroyShader)(Opal_Device device, Opal_Shader shader);
typedef Opal_Result (*PFN_opalDestroyBindsetLayout)(Opal_Device device, Opal_BindsetLayout bindset_layout);
typedef Opal_Result (*PFN_opalDestroyBindsetPool)(Opal_Device device, Opal_BindsetPool bindset_pool);
typedef Opal_Result (*PFN_opalDestroyPipelineLayout)(Opal_Device device, Opal_PipelineLayout pipeline_layout);
typedef Opal_Result (*PFN_opalDestroyPipeline)(Opal_Device device, Opal_Pipeline pipeline);
typedef Opal_Result (*PFN_opalDestroySwapchain)(Opal_Device device, Opal_Swapchain swapchain);
typedef Opal_Result (*PFN_opalDestroyDevice)(Opal_Device device);

typedef Opal_Result (*PFN_opalAllocateCommandBuffer)(Opal_Device device, Opal_CommandPool command_pool, Opal_CommandBuffer *command_buffer);
typedef Opal_Result (*PFN_opalFreeCommandBuffer)(Opal_Device device, Opal_CommandPool command_pool, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalResetCommandPool)(Opal_Device device, Opal_CommandPool command_pool);
typedef Opal_Result (*PFN_opalResetCommandBuffer)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalAllocateBindset)(Opal_Device device, Opal_BindsetPool bindset_pool, Opal_Bindset *bindset);
typedef Opal_Result (*PFN_opalFreeBindset)(Opal_Device device, Opal_BindsetPool bindset_pool, Opal_Bindset bindset);
typedef Opal_Result (*PFN_opalResetBindsetPool)(Opal_Device device, Opal_BindsetPool bindset_pool);
typedef Opal_Result (*PFN_opalMapBuffer)(Opal_Device device, Opal_Buffer buffer, void **ptr);
typedef Opal_Result (*PFN_opalUnmapBuffer)(Opal_Device device, Opal_Buffer buffer);
typedef Opal_Result (*PFN_opalUpdateBindset)(Opal_Device device, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings);
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

typedef Opal_Result (*PFN_opalCmdBeginGraphicsPass)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depthstencil_attachment);
typedef Opal_Result (*PFN_opalCmdEndGraphicsPass)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalCmdBeginComputePass)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalCmdEndComputePass)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalCmdBeginRaytracePass)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalCmdEndRaytracePass)(Opal_Device device, Opal_CommandBuffer command_buffer);
typedef Opal_Result (*PFN_opalCmdSetPipeline)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_Pipeline pipeline);
typedef Opal_Result (*PFN_opalCmdSetBindsets)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout, uint32_t num_bindsets, const Opal_Bindset *bindsets);
typedef Opal_Result (*PFN_opalCmdSetVertexBuffers)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers);
typedef Opal_Result (*PFN_opalCmdSetIndexBuffer)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer, Opal_IndexFormat index_format);
typedef Opal_Result (*PFN_opalCmdSetViewport)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_Viewport viewport);
typedef Opal_Result (*PFN_opalCmdSetScissor)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
typedef Opal_Result (*PFN_opalCmdDrawIndexedInstanced)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t base_index, uint32_t num_instances, uint32_t base_instance);
typedef Opal_Result (*PFN_opalCmdMeshletDispatch)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z);
typedef Opal_Result (*PFN_opalCmdComputeDispatch)(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z);
typedef Opal_Result (*PFN_opalCmdRaytraceDispatch)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTableView shader_table, uint32_t width, uint32_t height, uint32_t depth);
typedef Opal_Result (*PFN_opalCmdCopyBufferToBuffer)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size);
typedef Opal_Result (*PFN_opalCmdCopyBufferToTexture)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_TextureRegion dst);
typedef Opal_Result (*PFN_opalCmdCopyTextureToBuffer)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferView dst);
typedef Opal_Result (*PFN_opalCmdBufferTransitionBarrier)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after);
typedef Opal_Result (*PFN_opalCmdBufferQueueGrabBarrier)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue);
typedef Opal_Result (*PFN_opalCmdBufferQueueReleaseBarrier)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue);
typedef Opal_Result (*PFN_opalCmdTextureTransitionBarrier)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture, Opal_ResourceState state_before, Opal_ResourceState state_after);
typedef Opal_Result (*PFN_opalCmdTextureQueueGrabBarrier)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture, Opal_Queue queue);
typedef Opal_Result (*PFN_opalCmdTextureQueueReleaseBarrier)(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture, Opal_Queue queue);

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

	PFN_opalCreateSemaphore createSemaphore;
	PFN_opalCreateBuffer createBuffer;
	PFN_opalCreateTexture createTexture;
	PFN_opalCreateTextureView createTextureView;
	PFN_opalCreateSampler createSampler;
	PFN_opalCreateCommandPool createCommandPool;
	PFN_opalCreateShader createShader;
	PFN_opalCreateBindsetLayout createBindsetLayout;
	PFN_opalCreateBindsetPool createBindsetPool;
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
	PFN_opalDestroyCommandPool destroyCommandPool;
	PFN_opalDestroyShader destroyShader;
	PFN_opalDestroyBindsetLayout destroyBindsetLayout;
	PFN_opalDestroyBindsetPool destroyBindsetPool;
	PFN_opalDestroyPipelineLayout destroyPipelineLayout;
	PFN_opalDestroyPipeline destroyPipeline;
	PFN_opalDestroySwapchain destroySwapchain;
	PFN_opalDestroyDevice destroyDevice;

	PFN_opalAllocateCommandBuffer allocateCommandBuffer;
	PFN_opalFreeCommandBuffer freeCommandBuffer;
	PFN_opalResetCommandPool resetCommandPool;
	PFN_opalResetCommandBuffer resetCommandBuffer;
	PFN_opalAllocateBindset allocateBindset;
	PFN_opalFreeBindset freeBindset;
	PFN_opalResetBindsetPool resetBindsetPool;
	PFN_opalMapBuffer mapBuffer;
	PFN_opalUnmapBuffer unmapBuffer;
	PFN_opalUpdateBindset updateBindset;
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

	PFN_opalCmdBeginGraphicsPass cmdBeginGraphicsPass;
	PFN_opalCmdEndGraphicsPass cmdEndGraphicsPass;
	PFN_opalCmdBeginComputePass cmdBeginComputePass;
	PFN_opalCmdEndComputePass cmdEndComputePass;
	PFN_opalCmdBeginRaytracePass cmdBeginRaytracePass;
	PFN_opalCmdEndRaytracePass cmdEndRaytracePass;
	PFN_opalCmdSetPipeline cmdSetPipeline;
	PFN_opalCmdSetBindsets cmdSetBindsets;
	PFN_opalCmdSetVertexBuffers cmdSetVertexBuffers;
	PFN_opalCmdSetIndexBuffer cmdSetIndexBuffer;
	PFN_opalCmdSetViewport cmdSetViewport;
	PFN_opalCmdSetScissor cmdSetScissor;
	PFN_opalCmdDrawIndexedInstanced cmdDrawIndexedInstanced;
	PFN_opalCmdMeshletDispatch cmdMeshletDispatch;
	PFN_opalCmdComputeDispatch cmdComputeDispatch;
	PFN_opalCmdRaytraceDispatch cmdRaytraceDispatch;
	PFN_opalCmdCopyBufferToBuffer cmdCopyBufferToBuffer;
	PFN_opalCmdCopyBufferToTexture cmdCopyBufferToTexture;
	PFN_opalCmdCopyTextureToBuffer cmdCopyTextureToBuffer;
	PFN_opalCmdBufferTransitionBarrier cmdBufferTransitionBarrier;
	PFN_opalCmdBufferQueueGrabBarrier cmdBufferQueueGrabBarrier;
	PFN_opalCmdBufferQueueReleaseBarrier cmdBufferQueueReleaseBarrier;
	PFN_opalCmdTextureTransitionBarrier cmdTextureTransitionBarrier;
	PFN_opalCmdTextureQueueGrabBarrier cmdTextureQueueGrabBarrier;
	PFN_opalCmdTextureQueueReleaseBarrier cmdTextureQueueReleaseBarrier;
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

OPAL_APIENTRY Opal_Result opalDestroyInstance(Opal_Instance instance);
OPAL_APIENTRY Opal_Result opalDestroySurface(Opal_Instance instance, Opal_Surface surface);

OPAL_APIENTRY Opal_Result opalGetDeviceInfo(Opal_Device device, Opal_DeviceInfo *info);
OPAL_APIENTRY Opal_Result opalGetDeviceQueue(Opal_Device device, Opal_DeviceEngineType engine_type, uint32_t index, Opal_Queue *queue);

OPAL_APIENTRY Opal_Result opalCreateSemaphore(Opal_Device device, const Opal_SemaphoreDesc *desc, Opal_Semaphore *semaphore);
OPAL_APIENTRY Opal_Result opalCreateBuffer(Opal_Device device, const Opal_BufferDesc *desc, Opal_Buffer *buffer);
OPAL_APIENTRY Opal_Result opalCreateTexture(Opal_Device device, const Opal_TextureDesc *desc, Opal_Texture *texture);
OPAL_APIENTRY Opal_Result opalCreateTextureView(Opal_Device device, const Opal_TextureViewDesc *desc, Opal_TextureView *texture_view);
OPAL_APIENTRY Opal_Result opalCreateSampler(Opal_Device device, const Opal_SamplerDesc *desc, Opal_Sampler *sampler);
OPAL_APIENTRY Opal_Result opalCreateCommandPool(Opal_Device device, Opal_Queue queue, Opal_CommandPool *command_pool);
OPAL_APIENTRY Opal_Result opalCreateShader(Opal_Device device, const Opal_ShaderDesc *desc, Opal_Shader *shader);
OPAL_APIENTRY Opal_Result opalCreateBindsetLayout(Opal_Device device, uint32_t num_bindings, const Opal_BindsetLayoutBinding *bindings, Opal_BindsetLayout *bindset_layout);
OPAL_APIENTRY Opal_Result opalCreateBindsetPool(Opal_Device, Opal_BindsetLayout bindset_layout, uint32_t max_bindsets, Opal_BindsetPool *bindset_pool);
OPAL_APIENTRY Opal_Result opalCreatePipelineLayout(Opal_Device device, uint32_t num_bindset_layouts, const Opal_BindsetLayout *bindset_layouts, Opal_PipelineLayout *pipeline_layout);
OPAL_APIENTRY Opal_Result opalCreateGraphicsPipeline(Opal_Device device, const Opal_GraphicsPipelineDesc *desc, Opal_Pipeline *pipeline);
OPAL_APIENTRY Opal_Result opalCreateMeshletPipeline(Opal_Device device, const Opal_MeshletPipelineDesc *desc, Opal_Pipeline *pipeline);
OPAL_APIENTRY Opal_Result opalCreateComputePipeline(Opal_Device device, const Opal_ComputePipelineDesc *desc, Opal_Pipeline *pipeline);
OPAL_APIENTRY Opal_Result opalCreateRaytracePipeline(Opal_Device device, const Opal_RaytracePipelineDesc *desc, Opal_Pipeline *pipeline);
OPAL_APIENTRY Opal_Result opalCreateSwapchain(Opal_Device device, const Opal_SwapchainDesc *desc, Opal_Swapchain *swapchain);

OPAL_APIENTRY Opal_Result opalDestroySemaphore(Opal_Device device, Opal_Semaphore semaphore);
OPAL_APIENTRY Opal_Result opalDestroyBuffer(Opal_Device device, Opal_Buffer buffer);
OPAL_APIENTRY Opal_Result opalDestroyTexture(Opal_Device device, Opal_Texture texture);
OPAL_APIENTRY Opal_Result opalDestroyTextureView(Opal_Device device, Opal_TextureView texture_view);
OPAL_APIENTRY Opal_Result opalDestroySampler(Opal_Device device, Opal_Sampler sampler);
OPAL_APIENTRY Opal_Result opalDestroyCommandPool(Opal_Device device, Opal_CommandPool command_pool);
OPAL_APIENTRY Opal_Result opalDestroyShader(Opal_Device device, Opal_Shader shader);
OPAL_APIENTRY Opal_Result opalDestroyBindsetLayout(Opal_Device device, Opal_BindsetLayout bindset_layout);
OPAL_APIENTRY Opal_Result opalDestroyBindsetPool(Opal_Device, Opal_BindsetPool bindset_pool);
OPAL_APIENTRY Opal_Result opalDestroyPipelineLayout(Opal_Device device, Opal_PipelineLayout pipeline_layout);
OPAL_APIENTRY Opal_Result opalDestroyPipeline(Opal_Device device, Opal_Pipeline pipeline);
OPAL_APIENTRY Opal_Result opalDestroySwapchain(Opal_Device device, Opal_Swapchain swapchain);
OPAL_APIENTRY Opal_Result opalDestroyDevice(Opal_Device device);

OPAL_APIENTRY Opal_Result opalAllocateCommandBuffer(Opal_Device device, Opal_CommandPool command_pool, Opal_CommandBuffer *command_buffer);
OPAL_APIENTRY Opal_Result opalFreeCommandBuffer(Opal_Device device, Opal_CommandPool command_pool, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalResetCommandPool(Opal_Device device, Opal_CommandPool command_pool);
OPAL_APIENTRY Opal_Result opalResetCommandBuffer(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalAllocateBindset(Opal_Device device, Opal_BindsetPool bindset_pool, Opal_Bindset *bindset);
OPAL_APIENTRY Opal_Result opalFreeBindset(Opal_Device device, Opal_BindsetPool bindset_pool, Opal_Bindset bindset);
OPAL_APIENTRY Opal_Result opalResetBindsetPool(Opal_Device device, Opal_BindsetPool bindset_pool);
OPAL_APIENTRY Opal_Result opalMapBuffer(Opal_Device device, Opal_Buffer buffer, void **ptr);
OPAL_APIENTRY Opal_Result opalUnmapBuffer(Opal_Device device, Opal_Buffer buffer);
OPAL_APIENTRY Opal_Result opalUpdateBindset(Opal_Device device, Opal_Bindset bindset, uint32_t num_bindings, const Opal_BindsetBinding *bindings);
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

OPAL_APIENTRY Opal_Result opalCmdBeginGraphicsPass(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_color_attachments, const Opal_FramebufferAttachment *color_attachments, const Opal_FramebufferAttachment *depthstencil_attachment);
OPAL_APIENTRY Opal_Result opalCmdEndGraphicsPass(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalCmdBeginComputePass(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalCmdEndComputePass(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalCmdBeginRaytracePass(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalCmdEndRaytracePass(Opal_Device device, Opal_CommandBuffer command_buffer);
OPAL_APIENTRY Opal_Result opalCmdSetPipeline(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_Pipeline pipeline);
OPAL_APIENTRY Opal_Result opalCmdSetBindsets(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_PipelineLayout pipeline_layout, uint32_t num_bindsets, const Opal_Bindset *bindsets);
OPAL_APIENTRY Opal_Result opalCmdSetVertexBuffers(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_vertex_buffers, const Opal_BufferView *vertex_buffers);
OPAL_APIENTRY Opal_Result opalCmdSetIndexBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView index_buffer, Opal_IndexFormat index_format);
OPAL_APIENTRY Opal_Result opalCmdSetViewport(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_Viewport viewport);
OPAL_APIENTRY Opal_Result opalCmdSetScissor(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
OPAL_APIENTRY Opal_Result opalCmdDrawIndexedInstanced(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_indices, uint32_t base_index, uint32_t num_instances, uint32_t base_instance);
OPAL_APIENTRY Opal_Result opalCmdMeshletDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z);
OPAL_APIENTRY Opal_Result opalCmdComputeDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z);
OPAL_APIENTRY Opal_Result opalCmdRaytraceDispatch(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_ShaderBindingTableView shader_table, uint32_t width, uint32_t height, uint32_t depth);
OPAL_APIENTRY Opal_Result opalCmdCopyBufferToBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_BufferView dst, uint64_t size);
OPAL_APIENTRY Opal_Result opalCmdCopyBufferToTexture(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView src, Opal_TextureRegion dst);
OPAL_APIENTRY Opal_Result opalCmdCopyTextureToBuffer(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureRegion src, Opal_BufferView dst);
OPAL_APIENTRY Opal_Result opalCmdBufferTransitionBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_ResourceState state_before, Opal_ResourceState state_after);
OPAL_APIENTRY Opal_Result opalCmdBufferQueueGrabBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue);
OPAL_APIENTRY Opal_Result opalCmdBufferQueueReleaseBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_BufferView buffer, Opal_Queue queue);
OPAL_APIENTRY Opal_Result opalCmdTextureTransitionBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_ResourceState state_before, Opal_ResourceState state_after);
OPAL_APIENTRY Opal_Result opalCmdTextureQueueGrabBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue);
OPAL_APIENTRY Opal_Result opalCmdTextureQueueReleaseBarrier(Opal_Device device, Opal_CommandBuffer command_buffer, Opal_TextureView texture_view, Opal_Queue queue);
#endif

#ifdef __cplusplus
}
#endif
