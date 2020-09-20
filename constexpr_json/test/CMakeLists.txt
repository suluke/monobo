add_executable(basic basic.cc)

add_executable(static_unittests static_unittests.cc)

include(GenerateJsonHeader)

generate_json_header("${CMAKE_CURRENT_BINARY_DIR}/json_schema.h" "json_schema.json")
add_executable(json_schema json_schema_test.cc "${CMAKE_CURRENT_BINARY_DIR}/json_schema.h")
target_include_directories(json_schema PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(json_schema PRIVATE "-fconstexpr-steps=10000000")
endif()

add_subdirectory(JSONTestSuite)
