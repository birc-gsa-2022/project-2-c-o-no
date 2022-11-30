cmake_minimum_required (VERSION 3.21)
project (PROJECT)
enable_testing()

add_executable(st src/st.c src/suffix_tree.h src/suffix_tree.c)
add_executable(test_st test/test.c src/suffix_tree.c)
add_test(test test_st)

