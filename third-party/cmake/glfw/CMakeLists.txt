project( glfw_wrapper VERSION 3.4 LANGUAGES C )

set( ROOT_DIR "${CMAKE_BINARY_DIR}/deps/glfw" )

if ( NOT EXISTS "${ROOT_DIR}/.git" )

	file( REMOVE_RECURSE "${ROOT_DIR}" )

	execute_process(
		COMMAND git clone --filter=blob:none --sparse https://github.com/glfw/glfw.git "${ROOT_DIR}"
	)

	execute_process(
		COMMAND git -C "${ROOT_DIR}" checkout 7b6aead9fb88b3623e3b3725ebb42670cbe4c579 # TAG 3.4
	)

	execute_process(
		COMMAND git -C "${ROOT_DIR}" sparse-checkout set --no-cone
			/CMake
			/deps
			/include
			/src
			/CMakeLists.txt
	)
endif()


set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE )
set( GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE )
set( GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE )
set( GLFW_INSTALL        OFF CACHE BOOL "" FORCE )


if ( _WAYLAND )
    set(GLFW_BUILD_WAYLAND ON  CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11     OFF CACHE BOOL "" FORCE)
else()
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11     ON  CACHE BOOL "" FORCE)
endif()

add_subdirectory(
	${ROOT_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/glfw
)
