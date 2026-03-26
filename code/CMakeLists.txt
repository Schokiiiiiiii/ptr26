cmake_minimum_required(VERSION 3.20)

project(labo1 C)

set(CMAKE_CXX_FLAGS "-O2 -lpthread")

add_executable(cpu_loop cpu_loop.c)
add_executable(priority priority.c)
add_executable(get_cpu_number get_cpu_number.c)

