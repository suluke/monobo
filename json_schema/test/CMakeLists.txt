# Set up directory-wide variables
# MUST be before any add_subdirectory calls
find_path(CONSTEXPR_JSON_PROJECT_ROOT
  NAMES include/constexpr_json/document.h
  REQUIRED
  PATHS ${CMAKE_SOURCE_DIR}/constexpr_json
)

add_subdirectory(JSON-Schema-Test-Suite)

include(${CONSTEXPR_JSON_PROJECT_ROOT}/cmake/GenerateJsonHeader.cmake)
generate_json_header("${CMAKE_CURRENT_BINARY_DIR}/meta/core.json.h" "../res/meta/core.json")
generate_json_header("${CMAKE_CURRENT_BINARY_DIR}/meta/applicator.json.h" "../res/meta/applicator.json")
generate_json_header("${CMAKE_CURRENT_BINARY_DIR}/meta/validation.json.h" "../res/meta/validation.json")
generate_json_header("${CMAKE_CURRENT_BINARY_DIR}/meta/meta-data.json.h" "../res/meta/meta-data.json")
generate_json_header("${CMAKE_CURRENT_BINARY_DIR}/meta/format.json.h" "../res/meta/format.json")
generate_json_header("${CMAKE_CURRENT_BINARY_DIR}/meta/content.json.h" "../res/meta/content.json")
generate_json_header("${CMAKE_CURRENT_BINARY_DIR}/schema.json.h" "../res/schema.json")
set(GENERATED_HEADERS
  "${CMAKE_CURRENT_BINARY_DIR}/meta/core.json.h"
  "${CMAKE_CURRENT_BINARY_DIR}/meta/applicator.json.h"
  "${CMAKE_CURRENT_BINARY_DIR}/meta/validation.json.h"
  "${CMAKE_CURRENT_BINARY_DIR}/meta/meta-data.json.h"
  "${CMAKE_CURRENT_BINARY_DIR}/meta/format.json.h"
  "${CMAKE_CURRENT_BINARY_DIR}/meta/content.json.h"
  "${CMAKE_CURRENT_BINARY_DIR}/schema.json.h"
)

add_executable(json_schema_basic_test basic.cc ${GENERATED_HEADERS})
target_link_libraries(json_schema_basic_test PRIVATE json_schema)
target_include_directories(json_schema_basic_test
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)
add_test(NAME json_schema_basic_test COMMAND json_schema_basic_test)
