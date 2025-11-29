cmake_minimum_required(VERSION 3.10)
set(TARGET 06_gpu_particles)

# ==================================================================================================
# Variables
# ==================================================================================================
set(OPAL_PLATFORM_DEFINES "")

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
	${CMAKE_CURRENT_SOURCE_DIR}/app.cpp
)

file(GLOB HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/app.h
)

if (WIN32)
	file(GLOB WIN32_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/main.win.cpp
	)

	list(APPEND SOURCES ${WIN32_SOURCES})
endif()

if (APPLE)
	file(GLOB OSX_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/main.osx.mm
	)

	list(APPEND SOURCES ${OSX_SOURCES})
endif()

if (EMSCRIPTEN)
	file(GLOB WASM_SOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/main.wasm.cpp
	)

	list(APPEND SOURCES ${WASM_SOURCES})
endif()

# ==================================================================================================
# Shaders
# ==================================================================================================
set(COMPILED_SHADERS "")
set(GLSL_SHADERS "")
set(HLSL_SHADERS "")
set(METAL_SHADERS "")

if (WIN32)
	file(GLOB WIN32_GLSL_SHADERS
		${CMAKE_CURRENT_SOURCE_DIR}/shaders/vulkan/*.glsl
	)

	file(GLOB WIN32_HLSL_SHADERS
		${CMAKE_CURRENT_SOURCE_DIR}/shaders/directx12/*.hlsl
	)

	list(APPEND GLSL_SHADERS ${WIN32_GLSL_SHADERS})
	list(APPEND HLSL_SHADERS ${WIN32_HLSL_SHADERS})
endif()

if (APPLE)
	file(GLOB OSX_METAL_SHADERS
		${CMAKE_CURRENT_SOURCE_DIR}/shaders/metal/*.metal
	)

	list(APPEND METAL_SHADERS ${OSX_METAL_SHADERS})
endif()

foreach(SHADER ${GLSL_SHADERS})
	compile_glsl(${SHADER})
	list(APPEND COMPILED_SHADERS ${COMPILE_GLSL_RESULT_SPV})
endforeach()

foreach(SHADER ${HLSL_SHADERS})
	compile_hlsl(${SHADER})
	list(APPEND COMPILED_SHADERS ${COMPILE_HLSL_RESULT_CSO})
endforeach()

foreach(SHADER ${METAL_SHADERS})
	compile_metal(${SHADER})
	list(APPEND COMPILED_SHADERS ${COMPILE_METAL_RESULT_MLIB})
endforeach()

# ==================================================================================================
# Target
# ==================================================================================================
add_executable(${TARGET} ${SOURCES} ${HEADERS})
add_custom_target(${TARGET}_Shaders ALL DEPENDS ${COMPILED_SHADERS})

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
		-sALLOW_MEMORY_GROWTH
		--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/shaders/webgpu/emit.comp.wgsl@samples/06_gpu_particles/shaders/webgpu/emit.comp.wgsl
		--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/shaders/webgpu/simulate.comp.wgsl@samples/06_gpu_particles/shaders/webgpu/simulate.comp.wgsl
		--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/shaders/webgpu/render.vert.wgsl@samples/06_gpu_particles/shaders/webgpu/render.vert.wgsl
		--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/shaders/webgpu/render.frag.wgsl@samples/06_gpu_particles/shaders/webgpu/render.frag.wgsl
		--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/main.wasm.html
	")
endif()

if (APPLE)
	# TODO: for some reason CMake is unable to properly set rpath, so we set it manually
	set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "-Wl,-rpath,@executable_path,-rpath,@loader_path,-rpath,@executable_path/../lib")
endif()

# ==================================================================================================
# Libraries
# ==================================================================================================
target_link_libraries(${TARGET} PUBLIC opal)

if (APPLE)
	target_link_libraries(${TARGET} PRIVATE "-framework Cocoa -framework QuartzCore")
endif()

# ==================================================================================================
# Custom commands
# ==================================================================================================

# ==================================================================================================
# Installation
# ==================================================================================================
if (EMSCRIPTEN)
	install(
		FILES
		"$<TARGET_FILE_DIR:${TARGET}>/$<TARGET_FILE_BASE_NAME:${TARGET}>.js"
		"$<TARGET_FILE_DIR:${TARGET}>/$<TARGET_FILE_BASE_NAME:${TARGET}>.wasm"
		"$<TARGET_FILE_DIR:${TARGET}>/$<TARGET_FILE_BASE_NAME:${TARGET}>.html"
		"$<TARGET_FILE_DIR:${TARGET}>/$<TARGET_FILE_BASE_NAME:${TARGET}>.data"
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
