cmake_minimum_required (VERSION 3.21)
project (PROJECT)
enable_testing()

add_executable(st src/st.c)
add_test(st st)

