cmake_minimum_required(VERSION 3.16)

project(nanokit)

add_subdirectory(extern)

if (WIN32)
  set(BACKEND_SRC
        src/backend/win32/win32.c
    )

  set(BACKEND_LIBS
        opengl32
        dwmapi
    )
elseif(LINUX)

  set(BACKEND_SRC
        src/backend/glfw/glfw.c
    )

  set(BACKEND_LIBS
        glfw
        EGL
        GL
        m
        fontconfig

    )

  set(BACKEND_INCLUDES

    )
elseif(APPLE)

  enable_language(OBJC)

  set(BACKEND_SRC
        src/backend/cocoa/cocoa.m
    )

  set(BACKEND_LIBS
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework CoreVideo"
        "-framework QuartzCore"
    )

  set(BACKEND_INCLUDES

    )
endif()

add_library(nanokit STATIC
    src/lib.c
    src/resource/resource.c
    src/ui/ui.c
    src/ui/view/view.c
    src/ui/label/label.c
    src/ui/button/button.c
    src/ui/menu/menu.c
    src/ui/menubar/menubar.c
    src/ui/workbench/workbench.c

    ${BACKEND_SRC}
)

target_include_directories(nanokit PUBLIC
    include
)

target_include_directories(nanokit PRIVATE
    src
    extern
    ${BACKEND_INCLUDES}
)

target_link_libraries(nanokit PUBLIC
    glad
    nanovg
    ${BACKEND_LIBS}
)


# Embed Resources function courtesy of shir0areed on GitHub
function(embed_resources target)
  set(script_path "${CMAKE_CURRENT_BINARY_DIR}/anything_to_c.cmake")
  file(WRITE  "${script_path}" "file(READ \${CMAKE_ARGV3} buf HEX)\n")
  file(APPEND "${script_path}" "string(REGEX REPLACE \"([0-9a-f][0-9a-f])\" \"0x\\\\1, \" buf \${buf})\n")
  file(APPEND "${script_path}" "file(WRITE \${CMAKE_ARGV4} \"const unsigned char \${CMAKE_ARGV5}[] = { \${buf}0x00 };\\n\")\n")
  file(APPEND "${script_path}" "file(APPEND \${CMAKE_ARGV4} \"const unsigned \${CMAKE_ARGV6} = sizeof(\${CMAKE_ARGV5}) - 1;\\n\")\n")
  foreach(res_path ${ARGN})
    string(MAKE_C_IDENTIFIER ${res_path} identifier)
    set(src_path "${CMAKE_CURRENT_SOURCE_DIR}/${res_path}")
    set(dst_path "${CMAKE_CURRENT_BINARY_DIR}/${identifier}.c")
    set(anything_to_c ${CMAKE_COMMAND} -P ${script_path} ${src_path} ${dst_path} ${identifier} ${identifier}_size)
    add_custom_command(OUTPUT ${dst_path} COMMAND ${anything_to_c} DEPENDS ${src_path} VERBATIM)
    target_sources(${target} PRIVATE ${src_path} ${dst_path} )
  endforeach()
endfunction()
