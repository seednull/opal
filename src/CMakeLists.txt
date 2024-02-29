cmake_minimum_required(VERSION 3.10)
set(TARGET opal)

project(${TARGET} LANGUAGES C OBJC)

# ==================================================================================================
# Options
# ==================================================================================================

# ==================================================================================================
# Variables
# ==================================================================================================
set(OPAL_HAS_DIRECTX12 FALSE)
set(OPAL_HAS_WEBGPU FALSE)
set(OPAL_HAS_VULKAN FALSE)
set(OPAL_HAS_METAL FALSE)
set(OPAL_PLATFORM_DEFINES "")
set(OPAL_TARGET_TYPE SHARED)

if (WIN32)
	set(OPAL_HAS_DIRECTX12 ${OPAL_BUILD_WITH_DIRECTX12})
	set(OPAL_HAS_VULKAN ${OPAL_BUILD_WITH_VULKAN})
	set(OPAL_PLATFORM_DEFINES OPAL_PLATFORM_WIN32 NOMINMAX)
endif()

if (APPLE)
	set(OPAL_HAS_METAL ${OPAL_BUILD_WITH_METAL})
	set(OPAL_PLATFORM_DEFINES OPAL_PLATFORM_MACOS)
endif()

if (EMSCRIPTEN)
	set(OPAL_HAS_WEBGPU ${OPAL_BUILD_WITH_WEBGPU})
	set(OPAL_PLATFORM_DEFINES OPAL_PLATFORM_WEBGPU)
	set(OPAL_TARGET_TYPE STATIC)
endif()

# ==================================================================================================
# Dependencies
# ==================================================================================================

# ==================================================================================================
# Sources
# ==================================================================================================
file(GLOB SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/null/*.c
)

file(GLOB HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/null/*.h
)

if (OPAL_HAS_DIRECTX12)
	file(GLOB DIRECTX12_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/directx12/*.c
	)

	file(GLOB DIRECTX12_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/directx12/*.h
	)

	list(APPEND SOURCES ${DIRECTX12_SOURCES})
	list(APPEND HEADERS ${DIRECTX12_HEADERS})
	list(APPEND OPAL_PLATFORM_DEFINES OPAL_BACKEND_DIRECTX12)
else()
	list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/stubs/directx12_stub.c)
endif()

if (OPAL_HAS_WEBGPU)
	file(GLOB WEBGPU_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/webgpu/*.c
	)

	file(GLOB WEBGPU_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/webgpu/*.h
	)

	list(APPEND SOURCES ${WEBGPU_SOURCES})
	list(APPEND HEADERS ${WEBGPU_HEADERS})
	list(APPEND OPAL_PLATFORM_DEFINES OPAL_BACKEND_WEBGPU)
else()
	list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/stubs/webgpu_stub.c)
endif()

if (OPAL_HAS_VULKAN)
	file(GLOB VULKAN_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.c
		${OPAL_DIR_THIRDPARTY}/volk/include/volk.c
	)

	file(GLOB VULKAN_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.h
	)

	list(APPEND SOURCES ${VULKAN_SOURCES})
	list(APPEND HEADERS ${VULKAN_HEADERS})
	list(APPEND OPAL_PLATFORM_DEFINES OPAL_BACKEND_VULKAN)
else()
	list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/stubs/vulkan_stub.c)
endif()

if (OPAL_HAS_METAL)
	file(GLOB METAL_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/metal/*.m
	)

	file (GLOB METAL_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/metal/*.h
	)

	list(APPEND SOURCES ${METAL_SOURCES})
	list(APPEND HEADERS ${METAL_HEADERS})
	list(APPEND OPAL_PLATFORM_DEFINES OPAL_BACKEND_METAL)
else()
	list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/stubs/metal_stub.c)
endif()

# ==================================================================================================
# Target
# ==================================================================================================
add_library(${TARGET} ${OPAL_TARGET_TYPE} ${SOURCES} ${HEADERS})

set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})

set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})

set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})

set_target_properties(${TARGET} PROPERTIES DEBUG_POSTFIX d)

# ==================================================================================================
# Includes
# ==================================================================================================
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPAL_DIR_API})

if (OPAL_HAS_DIRECTX12)
	target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPAL_DIR_THIRDPARTY}/directx12/include)
endif()

if (OPAL_HAS_VULKAN)
	target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPAL_DIR_THIRDPARTY}/volk/include)
	target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPAL_DIR_THIRDPARTY}/vulkan/include)
endif()

# ==================================================================================================
# Preprocessor
# ==================================================================================================
target_compile_definitions(${TARGET} PRIVATE OPAL_SHARED_LIBRARY ${OPAL_PLATFORM_DEFINES})

# ==================================================================================================
# Libraries
# ==================================================================================================
if (OPAL_HAS_DIRECTX12)
	target_link_libraries(${TARGET} PRIVATE d3d12 dxgi) #TODO: get rid of these dependencies, load dll proc adresses manually
endif()

if (OPAL_HAS_METAL)
	target_link_libraries(${TARGET} PRIVATE "-framework Metal -framework Quartz -framework IOKit -framework Foundation")
endif()