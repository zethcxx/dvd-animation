project(stb_wrapper VERSION 1.0 LANGUAGES C)

set( ROOT_DIR "${CMAKE_BINARY_DIR}/deps/stb" )

if ( NOT EXISTS "${ROOT_DIR}/.git" )
    message(STATUS "Cloning stb into build directory: ${ROOT_DIR}")

    execute_process(
        COMMAND git clone --filter=blob:none --sparse https://github.com/nothings/stb.git "${ROOT_DIR}"
        RESULT_VARIABLE CLONE_RESULT
    )

    if( NOT CLONE_RESULT EQUAL 0 )
        message(FATAL_ERROR "Failed to clone stb")
    endif()

    execute_process(
        COMMAND git -C "${ROOT_DIR}" sparse-checkout set --no-cone
            /stb_image.h
            # /stb_image_write.h
    )
endif()

add_library( stb INTERFACE )

target_include_directories( stb SYSTEM INTERFACE "${ROOT_DIR}"     )
target_compile_definitions( stb INTERFACE STB_IMAGE_IMPLEMENTATION )
