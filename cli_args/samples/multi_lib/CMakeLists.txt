add_library(lib1 SHARED multi_lib_1.cc)
target_link_libraries(lib1 PRIVATE cli_args)

add_library(lib2 SHARED multi_lib_2.cc)
target_link_libraries(lib2 PRIVATE cli_args)

cli_args_add_sample(multi_lib
  SOURCES main.cc
  LINK_LIBRARIES lib1 lib2
)
