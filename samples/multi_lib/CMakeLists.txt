add_library(lib1 SHARED multi_lib_1.cc)
add_library(lib2 SHARED multi_lib_2.cc)
cli_args_add_sample(multi_lib main.cc)
target_link_libraries(multi_lib PRIVATE lib1 lib2)
