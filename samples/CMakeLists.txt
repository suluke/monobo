set(SAMPLES_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")

function(cli_args_add_sample TARGET)
  add_executable(${TARGET} ${ARGN})
  set_target_properties(${TARGET} PROPERTIES  RUNTIME_OUTPUT_DIRECTORY "${SAMPLES_BINARY_DIR}")
endfunction()

add_subdirectory(basic)
add_subdirectory(multi_lib)
add_subdirectory(multi_tu)
