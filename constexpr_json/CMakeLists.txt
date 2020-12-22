cmake_minimum_required(VERSION 3.8)
project(constexpr_json CXX C)
set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include_directories(include/)

add_subdirectory(lib)
add_subdirectory(tools)
add_subdirectory(test)
