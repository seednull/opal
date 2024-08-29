cmake_minimum_required(VERSION 3.10)
set(TARGET 04_triangle)

# ==================================================================================================
# Variables
# ==================================================================================================
if (WIN32)
	set(OPAL_PLATFORM_DEFINES OPAL_PLATFORM_WINDOWS NOMINMAX)
endif()

if (APPLE)
	set(OPAL_PLATFORM_DEFINES OPAL_PLATFORM_MACOS)
endif()

if (EMSCRIPTEN)
	set(OPAL_PLATFORM_DEFINES OPAL_PLATFORM_WEB)
endif()

# ==================================================================================================
# Sources
# ==================================================================================================
file(GLOB SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

file(GLOB HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# ==================================================================================================
# Target
# ==================================================================================================
add_executable(${TARGET} ${SOURCES} ${HEADERS})

set_target_properties(${TARGET} PROPERTIES DEBUG_POSTFIX d)

# ==================================================================================================
# Includes
# ==================================================================================================
target_include_directories(${TARGET} PUBLIC ${OPAL_API_DIR})

# ==================================================================================================
# Preprocessor
# ==================================================================================================
target_compile_definitions(${TARGET} PRIVATE ${OPAL_PLATFORM_DEFINES})

# ==================================================================================================
# Linker
# ==================================================================================================
if (EMSCRIPTEN)
	# TODO: could be better
	set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "
		--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vulkan/main.vert.spv@samples/04_triangle/shaders/vulkan/main.vert.spv
		--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vulkan/main.frag.spv@samples/04_triangle/shaders/vulkan/main.frag.spv
	")
endif()

# ==================================================================================================
# Libraries
# ==================================================================================================
target_link_libraries(${TARGET} PUBLIC opal)

# ==================================================================================================
# Custom commands
# ==================================================================================================

# ==================================================================================================
# Installation
# ==================================================================================================
if (EMSCRIPTEN)
	install(
		FILES
		"$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.js"
		"$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.wasm"
		"$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.html"
		"$<TARGET_FILE_DIR:${TARGET}>/${TARGET}.data"
		DESTINATION bin
	)
else()
	install(
		TARGETS ${TARGET}
		EXPORT ${TARGET}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		INCLUDES DESTINATION include
		PUBLIC_HEADER DESTINATION include
	)
endif()
