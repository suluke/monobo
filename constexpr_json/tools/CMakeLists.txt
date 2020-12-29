add_executable(json_format json_format.cc)
target_include_directories(json_format PRIVATE ../include)
target_link_libraries(json_format PRIVATE cli_args)

add_executable(json_validate json_validate.cc)
target_include_directories(json_validate PRIVATE ../include)
target_link_libraries(json_validate PRIVATE cli_args)
