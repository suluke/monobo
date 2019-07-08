cmake_minimum_required(VERSION 3.8)
project(monobo CXX C)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(cis)
add_subdirectory(cli_args)
add_subdirectory(constexpr_json)
