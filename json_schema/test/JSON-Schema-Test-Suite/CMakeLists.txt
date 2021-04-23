include(${CONSTEXPR_JSON_PROJECT_ROOT}/cmake/Download.cmake)

set(SCHEMA_TESTSUITE_DOWNLOAD_URL "https://github.com/json-schema-org/JSON-Schema-Test-Suite/archive/master.tar.gz")
set(SCHEMA_TESTSUITE_ARCHIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/JSON-Schema-Test-Suite.tar.gz")
set(SCHEMA_TESTSUITE_DIR "${CMAKE_CURRENT_BINARY_DIR}/JSON-Schema-Test-Suite-master")
set(SCHEMA_TESTSUITE_PACKAGE_JSON "${SCHEMA_TESTSUITE_DIR}/package.json")

# Step 1: Download archive from github
add_downloaded_file(
  "${SCHEMA_TESTSUITE_ARCHIVE_PATH}"
  "${SCHEMA_TESTSUITE_DOWNLOAD_URL}"
)

# Step 2: Extract downloaded archive
add_custom_command(
  OUTPUT "${SCHEMA_TESTSUITE_PACKAGE_JSON}"
  COMMAND "${CMAKE_COMMAND}" -E tar xfz "${SCHEMA_TESTSUITE_ARCHIVE_PATH}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS "${SCHEMA_TESTSUITE_ARCHIVE_PATH}"
)

add_custom_target(json-schema-test-suite DEPENDS ${SCHEMA_TESTSUITE_PACKAGE_JSON})

add_executable(json_schema_testsuite_driver driver.cc)
target_link_libraries(json_schema_testsuite_driver PRIVATE
  cli_args
  json_schema
  gtest
  # FIXME needs platform-specific coding
  stdc++fs
)

add_test(
  NAME json-schema-test-suite-download
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} -- json-schema-test-suite
)
set_tests_properties(json-schema-test-suite-download
  PROPERTIES FIXTURES_SETUP json-schema-test-suite
)

add_test(
  NAME json-schema-test-suite
  COMMAND json_schema_testsuite_driver ${SCHEMA_TESTSUITE_DIR}/tests/draft2019-09
)
set_tests_properties(json-schema-test-suite
  PROPERTIES
    FIXTURES_REQUIRED json-schema-test-suite
    WILL_FAIL ON
)
