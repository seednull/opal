cmake_minimum_required(VERSION 3.10)
project(opal VERSION 1.0.0 LANGUAGES C CXX OBJC)

# ==================================================================================================
# Global options
# ==================================================================================================
option(OPAL_BUILD_SAMPLES "Build samples" TRUE)
option(OPAL_BUILD_TESTS "Build tests" FALSE)
option(OPAL_BUILD_BENCHMARKS "Build benchmarks" FALSE)
option(OPAL_BUILD_WITH_DIRECTX12 "Build with DirectX 12 backend" TRUE)
option(OPAL_BUILD_WITH_WEBGPU "Build with WebGPU backend" TRUE)
option(OPAL_BUILD_WITH_VULKAN "Build with Vulkan backend" TRUE)
option(OPAL_BUILD_WITH_METAL "Build with Metal backend" TRUE)
option(OPAL_BUILD_WITH_VALIDATION_LAYERS "Build with Opal validation layers" FALSE)
option(OPAL_BUILD_WITH_VMA "Build with Vulkan Memory Allocator along with built-in memory allocator" FALSE)

# ==================================================================================================
# Global Variables
# ==================================================================================================
set(OPAL_DIR_THIRDPARTY ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
set(OPAL_DIR_API ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(OPAL_DIR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
	set(OPAL_PLATFORM windows)
	set(OPAL_ABI x64)
endif()

if (APPLE)
	set(OPAL_PLATFORM macos)
	set(OPAL_ABI x64) # FIXME: Is it always x64?
endif()

if (EMSCRIPTEN)
	set(OPAL_PLATFORM emscripten)
	set(OPAL_ABI wasm)
endif()

if (EMSCRIPTEN)
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

message("Platform: ${OPAL_PLATFORM}")
message("ABI: ${OPAL_ABI}")

# ==================================================================================================
# General compile flags
# ==================================================================================================
if (WIN32)
	set(CMAKE_C_FLAGS "/wd4996 /D_CRT_SECURE_NO_WARNINGS")
	set(CMAKE_CXX_FLAGS "/wd4996 /D_CRT_SECURE_NO_WARNINGS /EHsc /GR-")
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

if (OPAL_BUILD_TESTS)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	add_subdirectory(3rdparty/gtest)

	add_subdirectory(tests/heap)
	add_subdirectory(tests/pool)
endif()

if (OPAL_BUILD_BENCHMARKS)
	set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
	set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
	add_subdirectory(3rdparty/gbench)

	add_subdirectory(benchmarks/allocator)
endif()

if (OPAL_BUILD_SAMPLES)
	add_subdirectory(samples/01_enumerate_devices)
	add_subdirectory(samples/02_buffers)
	add_subdirectory(samples/03_textures)
	add_subdirectory(samples/04_triangle)
endif()
