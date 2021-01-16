cmake_minimum_required(VERSION 3.19)
project(json_schema CXX)
set(CMAKE_CXX_STANDARD 17)

find_path(GSL_INCLUDE_DIR
  NAMES gsl/span
  REQUIRED
  PATHS ${CMAKE_SOURCE_DIR}
)
add_library(json_schema INTERFACE)
target_include_directories(json_schema INTERFACE include ${GSL_INCLUDE_DIR})
target_link_libraries(json_schema INTERFACE constexpr_json)

add_subdirectory(lib)
add_subdirectory(test)
