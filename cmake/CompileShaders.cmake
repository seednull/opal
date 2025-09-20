# ==================================================================================================
# Functions
# ==================================================================================================
function(compile_glsl SRC)
	get_filename_component(FILE_NAME ${SRC} NAME_WLE)
	get_filename_component(FILE_EXT ${SRC} EXT)
	get_filename_component(FILE_DIRECTORY ${SRC} DIRECTORY)
	set(SPV_FILE ${FILE_DIRECTORY}/${FILE_NAME}.spv)

	list(APPEND REGEXPS "^\\.vert" "^\\.tesc" "^\\.tese" "^\\.geom" "^\\.comp"
						"^\\.frag" "^\\.mesh" "^\\.task" "^\\.rgen" "^\\.rint"
						"^\\.rahit" "^\\.rchit" "^\\.rmiss" "^\\.rcall")

	list(APPEND ENTRY_POINTS vertexMain main main main main
							 fragmentMain main main rayGenerationMain rayIntersectionMain
							 rayAnyHitMain rayClosestHitMain rayMissMain main)

	set(ENTRY "")

	foreach(VALUE IN ZIP_LISTS REGEXPS ENTRY_POINTS)
		if(FILE_EXT MATCHES ${VALUE_0})
			set(ENTRY ${VALUE_1})
			break()
		endif()
	endforeach()

	if(NOT ENTRY)
		message(FATAL_ERROR "No GLSL entry point found for ${SRC} (${FILE_EXT})")
	endif()

	add_custom_command(
		OUTPUT ${SPV_FILE}
		COMMAND glslangValidator --target-env vulkan1.3 -e ${ENTRY} --source-entrypoint main -o ${SPV_FILE} ${SRC}
		DEPENDS ${SRC}
		COMMENT "Compiling GLSL ${SRC} -> ${SPV_FILE}"
		VERBATIM
	)

	set(COMPILE_GLSL_RESULT_SPV ${SPV_FILE} PARENT_SCOPE)
endfunction()

function(compile_hlsl SRC)
	get_filename_component(FILE_NAME ${SRC} NAME_WLE)
	get_filename_component(FILE_EXT ${SRC} EXT)
	get_filename_component(FILE_DIRECTORY ${SRC} DIRECTORY)
	set(CSO_FILE ${FILE_DIRECTORY}/${FILE_NAME}.cso)

	list(APPEND REGEXPS "^\\.vert" "^\\.tesc" "^\\.tese" "^\\.geom" "^\\.comp"
						"^\\.frag" "^\\.mesh" "^\\.task" "^\\.rgen" "^\\.rint"
						"^\\.rahit" "^\\.rchit" "^\\.rmiss" "^\\.rcall")

	list(APPEND PROFILES vs_6_1 hs_6_1 ds_6_1 gs_6_1 cs_6_1
						 ps_6_1 ms_6_1 as_6_1 lib_6_3 lib_6_3
						 lib_6_3 lib_6_3 lib_6_3 lib_6_3)

	list(APPEND ENTRY_POINTS vertexMain main main main main
							 fragmentMain main main rayGenerationMain rayIntersectionMain
							 rayAnyHitMain rayClosestHitMain rayMissMain main)

	set(PROFILE "")
	set(ENTRY "")

	foreach(VALUE IN ZIP_LISTS REGEXPS PROFILES ENTRY_POINTS)
		if(FILE_EXT MATCHES ${VALUE_0})
			set(PROFILE ${VALUE_1})
			set(ENTRY ${VALUE_2})
			break()
		endif()
	endforeach()

	if(NOT PROFILE)
		message(FATAL_ERROR "No HLSL profile found for ${SRC} (${FILE_EXT})")
	endif()

	if(NOT ENTRY)
		message(FATAL_ERROR "No HLSL entry point found for ${SRC} (${FILE_EXT})")
	endif()

	add_custom_command(
		OUTPUT ${CSO_FILE}
		COMMAND dxc -T ${PROFILE} -E ${ENTRY} -Fo ${CSO_FILE} ${SRC}
		DEPENDS ${SRC}
		COMMENT "Compiling HLSL ${SRC} -> ${CSO_FILE}"
		VERBATIM
	)
	set(COMPILE_HLSL_RESULT_CSO ${CSO_FILE} PARENT_SCOPE)
endfunction()

function(compile_metal SRC)
	get_filename_component(FILE_NAME ${SRC} NAME_WLE)
	get_filename_component(FILE_EXT ${SRC} EXT)
	get_filename_component(FILE_DIRECTORY ${SRC} DIRECTORY)
	set(IR_FILE ${FILE_DIRECTORY}/${FILE_NAME}.ir)
	set(MLIB_FILE ${FILE_DIRECTORY}/${FILE_NAME}.metallib)

	add_custom_command(
		OUTPUT ${MLIB_FILE}
		COMMAND xcrun metal -c ${SRC} -o ${IR_FILE} -frecord-sources -gline-tables-only
		COMMAND xcrun metallib ${IR_FILE} -o ${MLIB_FILE}
		COMMAND ${CMAKE_COMMAND} -E rm -f ${IR_FILE}
		DEPENDS ${SRC}
		COMMENT "Compiling Metal ${SRC} -> ${MLIB_FILE}"
		VERBATIM
	)
	set(COMPILE_METAL_RESULT_MLIB ${MLIB_FILE} PARENT_SCOPE)
endfunction()
