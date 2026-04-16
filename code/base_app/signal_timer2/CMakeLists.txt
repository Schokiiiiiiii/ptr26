include_directories(../../de1soc_io)

add_executable(signal_timer2 signal_timer2.c)

target_link_libraries(signal_timer2 pthread de1soc_io)