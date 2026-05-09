cmake_minimum_required(VERSION 3.16)

add_library(glad STATIC
    glad/glad.c
)

target_include_directories(glad PUBLIC
    glad/include
)
