include_directories(../../de1soc_io)

add_executable(pthread_timer pthread_timer.c)

target_link_libraries(pthread_timer pthread de1soc_io)