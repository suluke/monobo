add_executable(cjson_basic basic.cc)
add_test(NAME cjson_basic COMMAND cjson_basic)

add_executable(cjson_static_unittests static_unittests.cc)
add_test(NAME cjson_static_unittests COMMAND cjson_static_unittests)

include(GenerateJsonHeader)
generate_json_header("${CMAKE_CURRENT_BINARY_DIR}/json_schema.h" "json_schema.json")
add_executable(cjson_json_schema_test json_schema_test.cc "${CMAKE_CURRENT_BINARY_DIR}/json_schema.h")
target_include_directories(cjson_json_schema_test PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(cjson_json_schema_test PRIVATE "-fconstexpr-steps=10000000")
endif()
add_test(NAME cjson_json_schema_test COMMAND cjson_json_schema_test)

add_executable(cjson_printer_test printer_test.cc "${CMAKE_CURRENT_BINARY_DIR}/json_schema.h")
target_include_directories(cjson_printer_test PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
add_test(NAME cjson_printer_test COMMAND cjson_printer_test)

add_subdirectory(JSONTestSuite)
