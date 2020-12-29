cmake_minimum_required(VERSION 3.8)
project(cli_args CXX C)
set(CMAKE_CXX_STANDARD 17)

find_path(GSL_INCLUDE_DIR
  NAMES gsl/span
  REQUIRED
  PATHS ${CMAKE_SOURCE_DIR}
)
add_library(cli_args INTERFACE)
target_include_directories(cli_args INTERFACE include "${GSL_INCLUDE_DIR}")

add_subdirectory(samples)
