include(${CONSTEXPR_JSON_PROJECT_ROOT}/cmake/Download.cmake)

set(SCHEMA_TESTSUITE_DOWNLOAD_URL "https://github.com/json-schema-org/JSON-Schema-Test-Suite/archive/master.tar.gz")
set(SCHEMA_TESTSUITE_ARCHIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/JSON-Schema-Test-Suite.tar.gz")
set(SCHEMA_TESTSUITE_PACKAGE_JSON "${CMAKE_CURRENT_BINARY_DIR}/JSON-Schema-Test-Suite-master/package.json")

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
target_link_libraries(json_schema_testsuite_driver PRIVATE cli_args)
target_link_libraries(json_schema_testsuite_driver PRIVATE json_schema)
# FIXME needs platform-specific coding
target_link_libraries(json_schema_testsuite_driver PRIVATE stdc++fs)
