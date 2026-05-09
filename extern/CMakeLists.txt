cmake_minimum_required(VERSION 3.16)

# GLAD
add_library(glad STATIC
    glad/glad.c
)

target_include_directories(glad PUBLIC
    glad
)

# Freetype
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
add_subdirectory(freetype)

# NanoVG
add_library(nanovg STATIC
    nanovg/nanovg.c
)

target_include_directories(nanovg PUBLIC
    nanovg
)

target_compile_definitions(nanovg PRIVATE
    FONS_USE_FREETYPE
)

target_link_libraries(nanovg PUBLIC
    freetype
)
