add_executable(monobo_cliargs_test_basic basic.cc)
target_link_libraries(monobo_cliargs_test_basic PRIVATE
  cli_args
  gtest_main
)

gtest_discover_tests(monobo_cliargs_test_basic)
