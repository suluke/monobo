include(Download)

add_executable(jsontestsuite_driver EXCLUDE_FROM_ALL driver.cc)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(jsontestsuite_driver PRIVATE stdc++fs)
endif()

set(JSON_TESTSUITE_DOWNLOAD_URL "https://github.com/nst/JSONTestSuite/archive/master.tar.gz")
set(JSON_TESTSUITE_ARCHIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/JSONTestSuite.tar.gz")
set(JSON_TESTSUITE_EXE "${CMAKE_CURRENT_BINARY_DIR}/JSONTestSuite-master/run_tests.py")
set(JSON_TESTSUITE_EXE_PATCHED "${CMAKE_CURRENT_BINARY_DIR}/JSONTestSuite-master/run_tests_patched.py")
set(JSON_TESTSUITE_FILTER_JSON "${CMAKE_CURRENT_BINARY_DIR}/JSONTestSuite-master/jsontestsuite_programs.json")

# Step 1: Download archive from github
add_downloaded_file("${JSON_TESTSUITE_ARCHIVE_PATH}" "${JSON_TESTSUITE_DOWNLOAD_URL}")
# Step 2: Extract downloaded archive
add_custom_command(
  OUTPUT "${JSON_TESTSUITE_EXE}"
  COMMAND "${CMAKE_COMMAND}" -E tar xfz "${JSON_TESTSUITE_ARCHIVE_PATH}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS "${JSON_TESTSUITE_ARCHIVE_PATH}"
)
# Step 3: Patch runner script to learn about our test driver
add_custom_command(
  OUTPUT "${JSON_TESTSUITE_EXE_PATCHED}"
  COMMAND "${CMAKE_COMMAND}" -D INPUT_FILE="${JSON_TESTSUITE_EXE}" -D OUTPUT_FILE="${JSON_TESTSUITE_EXE_PATCHED}" -P "${CMAKE_CURRENT_LIST_DIR}/PatchScript.cmake"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS "${JSON_TESTSUITE_EXE}"
)
# Step 4: Create filter.json
add_custom_command(
  OUTPUT "${JSON_TESTSUITE_FILTER_JSON}"
  COMMAND "${CMAKE_COMMAND}" -D OUTPUT_FILE="${JSON_TESTSUITE_FILTER_JSON}" -P "${CMAKE_CURRENT_LIST_DIR}/CreateFilterJson.cmake"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

add_custom_target(JSONTestSuite
  COMMAND python3 "${JSON_TESTSUITE_EXE_PATCHED}" --filter="${JSON_TESTSUITE_FILTER_JSON}"
  DEPENDS "${JSON_TESTSUITE_EXE_PATCHED}" "${JSON_TESTSUITE_FILTER_JSON}"
  USES_TERMINAL
)
