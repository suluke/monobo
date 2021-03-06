set(THIS_CMAKE_FILE "${CMAKE_CURRENT_LIST_FILE}")

# This is the cmake script doing the work.
# It is called from a custom command generated by the "generate_json_header" function below
if (NOT PROJECT_SOURCE_DIR)
  if (NOT EXISTS "${JSON_PATH}")
    message(FATAL_ERROR "JSON file ${JSON_PATH} does not exist")
  endif()
  file(READ "${JSON_PATH}" JSON_CONTENT)
  file(WRITE "${HEADER_PATH}" "
    #ifndef USE_JSON_STRING\n
    #define USE_JSON_STRING(theJson)\n
    #endif\n
    USE_JSON_STRING(R\"{\"}\"\"{\"}(\n
  ")
  file(APPEND "${HEADER_PATH}" "${JSON_CONTENT}")
  file(APPEND "${HEADER_PATH}" "
  ){\"}\"\"{\"}\")
  #undef USE_JSON_STRING
  ")
  message(STATUS "READ FILE ")
endif()

# This is the public API of this file
function(generate_json_header HEADER_PATH JSON_PATH)
  add_custom_command(
    OUTPUT "${HEADER_PATH}"
    COMMAND "${CMAKE_COMMAND}" -D JSON_PATH="${JSON_PATH}" -D HEADER_PATH="${HEADER_PATH}" -P "${THIS_CMAKE_FILE}"
    DEPENDS "${JSON_PATH}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
  )
endfunction()
