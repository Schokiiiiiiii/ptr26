cmake_minimum_required(VERSION 3.20)

project(labo4 C)

add_compile_options(-O2 -Wall -Wextra)

add_executable(xbuf xbuf.c)
add_executable(proxy proxy.c)
add_executable(flags flags.c)

target_link_libraries(xbuf pthread evl)
target_link_libraries(proxy pthread evl m)
target_link_libraries(flags pthread evl)
