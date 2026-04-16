cmake_minimum_required(VERSION 3.20)

project(labo3 C)

add_compile_options(-O2 -Wall -Wextra)

add_library(de1soc_io STATIC de1soc_io/de1soc_io.c)

add_subdirectory(base_app)
add_subdirectory(rt_app)
