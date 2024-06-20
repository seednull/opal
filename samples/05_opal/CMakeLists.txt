cmake_minimum_required(VERSION 3.10)
set(TARGET 05_opal)

project(${TARGET} CXX)

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

set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OPAL_DIR_EXPORT}/${OPAL_PLATFORM}/${OPAL_ABI})

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
# Libraries
# ==================================================================================================
target_link_libraries(${TARGET} PUBLIC opal)

# ==================================================================================================
# Custom commands
# ==================================================================================================
