add_library(lib1 SHARED multi_lib_1.cc)
add_library(lib2 SHARED multi_lib_2.cc)
cli_args_add_sample(multi_lib
  SOURCES main.cc
  LINK_LIBRARIES lib1 lib2
)
