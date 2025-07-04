cmake_minimum_required(VERSION 3.10)
set(TARGET opal)

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
	set(OPAL_PLATFORM_DEFINES OPAL_PLATFORM_WIN32 OPAL_SHARED_LIBRARY NOMINMAX)
endif()

if (APPLE)
	set(OPAL_HAS_METAL ${OPAL_BUILD_WITH_METAL})
	set(OPAL_PLATFORM_DEFINES OPAL_PLATFORM_MACOS OPAL_SHARED_LIBRARY)
endif()

if (EMSCRIPTEN)
	set(OPAL_HAS_WEBGPU ${OPAL_BUILD_WITH_WEBGPU})
	set(OPAL_PLATFORM_DEFINES OPAL_PLATFORM_WEB)
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
	${CMAKE_CURRENT_SOURCE_DIR}/common/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/null/*.c
)

file(GLOB HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/null/*.h
)

file (GLOB PUBLIC_HEADERS
	${OPAL_DIR_API}/opal.h
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
endif()

if (OPAL_HAS_VULKAN)
	file(GLOB VULKAN_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_allocator.c
		${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_device.c
		${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_instance.c
		${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_utils.c
		${OPAL_DIR_THIRDPARTY}/volk/include/volk.c
	)

	if (WIN32)
		list(APPEND VULKAN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/vulkan_platform_win32.c)
		list(APPEND OPAL_PLATFORM_DEFINES VK_USE_PLATFORM_WIN32_KHR)
	endif()

	if (OPAL_BUILD_WITH_VMA)
		list(APPEND VULKAN_SOURCES ${OPAL_DIR_THIRDPARTY}/vma/include/vk_mem_alloc.cpp)
		list(APPEND OPAL_PLATFORM_DEFINES OPAL_HAS_VMA)
	endif()

	file(GLOB VULKAN_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/vulkan/*.h
	)

	list(APPEND SOURCES ${VULKAN_SOURCES})
	list(APPEND HEADERS ${VULKAN_HEADERS})
	list(APPEND OPAL_PLATFORM_DEFINES OPAL_BACKEND_VULKAN)
endif()

if (OPAL_HAS_METAL)
	file(GLOB METAL_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/metal/*.m
		${CMAKE_CURRENT_SOURCE_DIR}/metal/*.c
	)

	file (GLOB METAL_HEADERS
		${CMAKE_CURRENT_SOURCE_DIR}/metal/*.h
	)

	list(APPEND SOURCES ${METAL_SOURCES})
	list(APPEND HEADERS ${METAL_HEADERS})
	list(APPEND OPAL_PLATFORM_DEFINES OPAL_BACKEND_METAL)
endif()

# ==================================================================================================
# Target
# ==================================================================================================
add_library(${TARGET} ${OPAL_TARGET_TYPE} ${SOURCES} ${HEADERS} ${PUBLIC_HEADERS})

set_target_properties(${TARGET} PROPERTIES DEBUG_POSTFIX d)
set_target_properties(${TARGET} PROPERTIES PUBLIC_HEADER ${PUBLIC_HEADERS})

if (WIN32)
	set_target_properties(${TARGET} PROPERTIES PREFIX "")
	set_target_properties(${TARGET} PROPERTIES IMPORT_PREFIX "")
	set_target_properties(${TARGET} PROPERTIES IMPORT_SUFFIX ".lib")
else()
	set_target_properties(${TARGET} PROPERTIES IMPORT_SUFFIX ".a")
endif()

# ==================================================================================================
# Includes
# ==================================================================================================
target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPAL_DIR_API})

if (OPAL_HAS_VULKAN)
	target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPAL_DIR_THIRDPARTY}/volk/include)
	target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPAL_DIR_THIRDPARTY}/vulkan/include)
	target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPAL_DIR_THIRDPARTY}/vma/include)
endif()

# ==================================================================================================
# Preprocessor
# ==================================================================================================
target_compile_definitions(${TARGET} PRIVATE ${OPAL_PLATFORM_DEFINES})

# ==================================================================================================
# Linker
# ==================================================================================================
if (EMSCRIPTEN)
	target_link_options(${TARGET} PUBLIC "-sUSE_WEBGPU=1")
	target_link_options(${TARGET} PUBLIC "-sASYNCIFY")
endif()

# ==================================================================================================
# Libraries
# ==================================================================================================
if (OPAL_HAS_METAL)
	target_link_libraries(${TARGET} PRIVATE "-framework Metal -framework Foundation -framework IOKit -framework QuartzCore -framework CoreGraphics")
endif()

# ==================================================================================================
# Installation
# ==================================================================================================
install(
	TARGETS ${TARGET}
	EXPORT ${TARGET}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
	PUBLIC_HEADER DESTINATION include
)
