set(THIS_CMAKE_FILE "${CMAKE_CURRENT_LIST_FILE}")

if (NOT PROJECT_SOURCE_DIR)
  file(DOWNLOAD "${FILE_URL}" "${FILE_PATH}")
endif()

function(add_downloaded_file FILE_PATH FILE_URL)
  add_custom_command(
    OUTPUT "${FILE_PATH}"
    COMMAND "${CMAKE_COMMAND}" -D FILE_PATH="${FILE_PATH}" -D FILE_URL="${FILE_URL}" -P "${THIS_CMAKE_FILE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
  )
endfunction()
