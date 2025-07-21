cmake_minimum_required(VERSION 3.6)

project(NetCoreInterop LANGUAGES CXX)

# Global configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Binaries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Binaries")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -m64 -Wall -fvisibility=hidden")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -m64 -Wall -O3 -fvisibility=hidden")

# Platform-specific global configuration
if(WIN32)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mwindows")
endif()

# .NET library support
function(add_dotnet_assembly ASSEMBLY_NAME)
	# Setup
	set(OUTPUT_NAME_PATH ${CMAKE_SOURCE_DIR}/Build/${ASSEMBLY_NAME}/${ASSEMBLY_NAME})
	set(OUTPUT_DLL ${OUTPUT_NAME_PATH}.dll)
	set(OUTPUT_PDB ${OUTPUT_NAME_PATH}.pdb)
	set(OUTPUT_RUNTIMECONFIG ${OUTPUT_NAME_PATH}.runtimeconfig.json)

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(ASSEMBLY_BUILD_TYPE Debug)

	else()
		set(ASSEMBLY_BUILD_TYPE Release)
	endif()

	# Source files
	file(GLOB_RECURSE sources src/*.cs)

	add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/Build/${ASSEMBLY_NAME}/${ASSEMBLY_NAME}.dll
		COMMAND dotnet build ${CMAKE_CURRENT_SOURCE_DIR}/${ASSEMBLY_NAME}.csproj
		-c ${ASSEMBLY_BUILD_TYPE} -o ${CMAKE_SOURCE_DIR}/Build/${ASSEMBLY_NAME}
		COMMAND exit
		DEPENDS ${ASSEMBLY_NAME}.csproj ${sources}
		COMMENT "Building ${ASSEMBLY_NAME}.dll"
	)

	add_custom_target(${ASSEMBLY_NAME} ALL DEPENDS ${OUTPUT_DLL})

	set_property(TARGET ${ASSEMBLY_NAME}
		APPEND PROPERTY ADDITIONAL_CLEAN_FILES
		${CMAKE_SOURCE_DIR}/Binaries/${ASSEMBLY_NAME}.dll
		${CMAKE_SOURCE_DIR}/Binaries/${ASSEMBLY_NAME}.runtimeconfig.json)

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set_property(TARGET ${ASSEMBLY_NAME} APPEND
			PROPERTY ADDITIONAL_CLEAN_FILES ${CMAKE_SOURCE_DIR}/Binaries/${ASSEMBLY_NAME}.pdb)
	endif()

	add_custom_command(TARGET ${ASSEMBLY_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${OUTPUT_DLL}
		${CMAKE_SOURCE_DIR}/Binaries
		VERBATIM)

	add_custom_command(TARGET ${ASSEMBLY_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${OUTPUT_RUNTIMECONFIG}
		${CMAKE_SOURCE_DIR}/Binaries
		VERBATIM)

	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_custom_command(TARGET ${ASSEMBLY_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${OUTPUT_PDB}
			${CMAKE_SOURCE_DIR}/Binaries
			VERBATIM)

	else()
		if(EXISTS ${OUTPUT_PDB})
			add_custom_command(TARGET ${ASSEMBLY_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_SOURCE_DIR}/Binaries/${ASSEMBLY_NAME}.pdb VERBATIM)
		endif()
	endif()
endfunction()

# Projects
add_subdirectory("${CMAKE_SOURCE_DIR}/Interop.Core")
add_subdirectory("${CMAKE_SOURCE_DIR}/InteropLib")
add_subdirectory("${CMAKE_SOURCE_DIR}/Sandbox")