cmake_minimum_required(VERSION 3.10)
project(opal VERSION 1.0.0)

# ==================================================================================================
# Global options
# ==================================================================================================
option(OPAL_BUILD_SAMPLES "Build samples" TRUE)
option(OPAL_BUILD_WITH_DIRECTX12 "Build with DirectX 12 backend" TRUE)
option(OPAL_BUILD_WITH_WEBGPU "Build with WebGPU backend" TRUE)
option(OPAL_BUILD_WITH_VALIDATION_LAYERS "Build with Opal validation layers" FALSE)

# ==================================================================================================
# Global Variables
# ==================================================================================================
set(OPAL_DIR_THIRDPARTY ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
set(OPAL_DIR_EXPORT ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(OPAL_DIR_API ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (WIN32)
	set(OPAL_PLATFORM windows)
	set(OPAL_ABI x64)
endif()

if (EMSCRIPTEN)
	set(OPAL_PLATFORM emscripten)
	set(OPAL_ABI wasm)
endif()

if (EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# ==================================================================================================
# General compile flags
# ==================================================================================================
if (WIN32)
	set(CMAKE_C_FLAGS "/wd4996")
endif()

if (EMSCRIPTEN)
	add_link_options("-sUSE_WEBGPU=1")
	add_link_options("-sASYNCIFY")
endif()

# ==================================================================================================
# General includes
# ==================================================================================================

# ==================================================================================================
# General libraries
# ==================================================================================================

# ==================================================================================================
# Sub-projects
# ==================================================================================================
add_subdirectory(src)

if (OPAL_BUILD_SAMPLES)
	add_subdirectory(samples/01_enumerate_devices)
endif()
