cmake_minimum_required(VERSION 3.20)

project(labo1 C)

set(CMAKE_CXX_FLAGS "-O2 -lpthread")

add_executable(gettimeofday gettimeofday.c)
add_executable(gettimeofday2 gettimeofday2.c)
add_executable(signal_timer signal_timer.c)
add_executable(signal_timer2 signal_timer2.c)
