cmake_minimum_required(VERSION 3.8)
project(monobo CXX C)
set(CMAKE_CXX_STANDARD 17)

if (MSVC)
  add_compile_options(/W4 /WX)
else ()
  add_compile_options(-Wall -Wextra -pedantic -Werror -Wno-unused-parameter)
endif ()


add_subdirectory(cis)
add_subdirectory(cli_args)
add_subdirectory(constexpr_json)
