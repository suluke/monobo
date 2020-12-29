set(SAMPLES_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")

function(cli_args_add_sample TARGET_NAME)
  set(options "")
  set(oneValueArgs "")
  set(multiValueArgs SOURCES LINK_LIBRARIES)
  cmake_parse_arguments(SAMPLE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (SAMPLE_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unidentified argument: ${SAMPLE_UNPARSED_ARGUMENTS}")
  endif()

  set(TARGET "${PROJECT_NAME}_${TARGET_NAME}")
  add_executable(${TARGET} ${SAMPLE_SOURCES})
  target_link_libraries(${TARGET} PRIVATE cli_args)
  set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SAMPLES_BINARY_DIR}"
    OUTPUT_NAME "${TARGET_NAME}"
  )
  target_link_libraries(${TARGET} PRIVATE ${SAMPLE_LINK_LIBRARIES})
endfunction()

add_subdirectory(basic)
add_subdirectory(multi_lib)
add_subdirectory(multi_tu)
