cmake_minimum_required(VERSION 3.28)
project(os_labs C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Wextra -Werror -pedantic)

add_executable(os_labs lab1/parent.c)
add_executable(child lab1/child.c)
