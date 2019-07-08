cmake_minimum_required(VERSION 3.8)
project(constexpr_json CXX C)
set(CMAKE_CXX_STANDARD 17)

include_directories(include/)

add_executable(json_schema test/json_schema_test.cc)