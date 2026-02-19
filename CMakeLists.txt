cmake_minimum_required( VERSION 3.20 )

project( DvdAnim
	VERSION     0.1.0
	DESCRIPTION "A recreation of the classic DVD screensaver"
	LANGUAGES   CXX
)


# ------------------------------------------------------------
# CMAKE DEFAULT CONFIG
# ------------------------------------------------------------
if( NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES )
	set( CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE )
endif()


find_program( NINJA_EXE NAMES ninja ninja-build )
if ( NOT CMAKE_GENERATOR AND NINJA_EXE )
	set( CMAKE_GENERATOR "Ninja" CACHE STRING "CMake generator" FORCE )
endif()

# ------------------------------------------------------------
# CONSTANTS
# ------------------------------------------------------------
set( ROOT_DIR        "${CMAKE_CURRENT_SOURCE_DIR}" )
set( INCLUDE_DIR     "${ROOT_DIR}/include"         )
set( SOURCES_DIR     "${ROOT_DIR}/src"             )
set( TARGET_BIN_DIR  "${ROOT_DIR}/build/bin"       )
set( MAIN_TARGET     zlanim                        )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )

# ------------------------------------------------------------
# FLAGS CONFIG
# ------------------------------------------------------------
option( _NATIVE_BIN  "" OFF )
option( _STATIC_BIN  "" OFF )

include( ${ROOT_DIR}/cmake/config_flags.cmake )

# ------------------------------------------------------------
# DEPENDENCIES
# ------------------------------------------------------------
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/third-party )

# ------------------------------------------------------------
# MAIN TARGET CONFIG
# ------------------------------------------------------------
add_executable( ${MAIN_TARGET}
    ${ROOT_DIR}/app/main.cpp
)


target_include_directories( ${MAIN_TARGET}
PRIVATE
	${INCLUDE_DIR}
)


target_compile_definitions( ${MAIN_TARGET}
PRIVATE
	_GLIBCXX_ASSERTIONS

	$<$<CONFIG:Debug>:
		DEBUG=1
	>

	$<$<CONFIG:Release>:
		_FORTIFY_SOURCE=2
		NDEBUG=1
	>

	$<$<PLATFORM_ID:Windows>:
		WIN32_LEAN_AND_MEAN=1
		NOMINMAX=1
		UNICODE=1
		_UNICODE=1
	>
)


target_compile_options( ${MAIN_TARGET}
PRIVATE
	$<$<CONFIG:Debug>:
		${VALID_SANITIZER_FLAGS}
		${CUSTOM_DEBUG_FLAGS}
	>

	$<$<CONFIG:Release>:${CUSTOM_RELEASE_FLAGS}>
)


target_link_options( ${MAIN_TARGET}
PRIVATE
	$<$<CONFIG:Debug>:
		${VALID_SANITIZER_FLAGS}
		${CUSTOM_LINK_DEBUG_FLAGS}
	>

	$<$<CONFIG:Release>:
		${CUSTOM_LINK_RELEASE_FLAGS}
	>
)

target_link_libraries( ${MAIN_TARGET}
PRIVATE
	ExtraDeps
)

target_compile_features( ${MAIN_TARGET}
PRIVATE
	cxx_std_23
)


set_target_properties( ${MAIN_TARGET}
PROPERTIES
	CXX_EXTENSIONS           OFF
	CXX_STANDARD_REQUIRED    ON
	EXPORT_COMPILE_COMMANDS  ON
	RUNTIME_OUTPUT_DIRECTORY ${TARGET_BIN_DIR}
)
