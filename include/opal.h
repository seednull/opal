#pragma once

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

#define OPAL_DEFINE_HANDLE(TYPE) typedef uint32_t TYPE;

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handles
OPAL_DEFINE_HANDLE(Opal_Instance);
OPAL_DEFINE_HANDLE(Opal_Device);

// Enums
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

// Structs

// Function pointers
typedef void (*PFN_opalCreateInstance)(Opal_Api api, OpalInstance *instance);
typedef void (*PFN_opalDestroyInstance)(Opal_Instance instance);

// API
OPAL_APIENTRY PFN_opalCreateInstance opalCreateInstance;
OPAL_APIENTRY PFN_opalDestroyInstance opalDestroyInstance;

OPAL_APIENTRY int opalInit();

#ifdef __cplusplus
}
#endif
