project( third_party LANGUAGES CXX C )
set( ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

# ---------------- SYSTEM ----------------
find_package( OpenGL REQUIRED )

# --------------- EXTERNAL ---------------
add_subdirectory( ${ROOT_DIR}/cmake/glfw )
add_subdirectory( ${ROOT_DIR}/cmake/stb  )

add_library( ExtraDeps INTERFACE )
target_link_libraries( ExtraDeps INTERFACE
    OpenGL::GL
    glfw
    stb
)
