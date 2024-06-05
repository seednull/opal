cmake_minimum_required(VERSION 3.10)
set(TARGET test_pool)

project(${TARGET} CXX)

# ==================================================================================================
# Variables
# ==================================================================================================

# ==================================================================================================
# Sources
# ==================================================================================================
file(GLOB SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${OPAL_DIR_SRC}/common/pool.c
)

file(GLOB HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${OPAL_DIR_SRC}/common/*.h
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
target_include_directories(${TARGET} PUBLIC ${OPAL_DIR_API})
target_include_directories(${TARGET} PUBLIC ${OPAL_DIR_SRC}/common)

# ==================================================================================================
# Preprocessor
# ==================================================================================================

# ==================================================================================================
# Libraries
# ==================================================================================================
target_link_libraries(${TARGET} PUBLIC gtest)

# ==================================================================================================
# Custom commands
# ==================================================================================================
