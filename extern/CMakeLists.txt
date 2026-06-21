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
# Brotli powers WOFF2 decompression (needed to load the embedded .woff2 icon
# font). FreeType gates its whole Brotli search behind `NOT FT_DISABLE_BROTLI`,
# so FT_REQUIRE_BROTLI alone does nothing while FT_DISABLE_BROTLI is ON — it must
# be forced OFF here, otherwise FT_CONFIG_OPTION_USE_BROTLI is never defined and
# FT_New_Memory_Face rejects the woff2.
set(FT_DISABLE_BROTLI OFF CACHE BOOL "" FORCE)
set(FT_REQUIRE_BROTLI ON CACHE BOOL "" FORCE)
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

if (LINUX)
  # GLFW
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
  add_subdirectory(glfw)
endif()
