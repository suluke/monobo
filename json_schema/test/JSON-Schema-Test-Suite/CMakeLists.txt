FetchContent_Declare(
  json_schema_testsuite
  GIT_REPOSITORY https://github.com/json-schema-org/JSON-Schema-Test-Suite.git
  GIT_TAG        master
)
FetchContent_Populate(json_schema_testsuite)

configure_file(defs.h.in json_schema_testsuite_driver_defs.h @ONLY)
add_executable(json_schema_testsuite_driver driver.cc json_schema_testsuite_driver_defs.h)
target_include_directories(json_schema_testsuite_driver PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(json_schema_testsuite_driver PRIVATE
  cli_args
  json_schema
  gtest
  # FIXME needs platform-specific coding
  stdc++fs
)

add_test(
  NAME json-schema-test-suite
  COMMAND json_schema_testsuite_driver ${json_schema_testsuite_SOURCE_DIR}/tests/draft2019-09
)
set_tests_properties(json-schema-test-suite
  PROPERTIES
    WILL_FAIL ON
)

gtest_discover_tests(json_schema_testsuite_driver
  TEST_PREFIX "json_schema/"
  PROPERTIES
    # WILL_FAIL ON
)
