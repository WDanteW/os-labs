cmake_minimum_required(VERSION 3.28)
project(os_labs C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

add_compile_options(-Wall -Wextra -Werror -pedantic)

add_executable(os_labs lab1/parent.c)
add_executable(child1 lab1/child.c)

add_executable(gaussian lab2/gaussian_threads.c)
target_link_libraries(gaussian PRIVATE pthread m)

add_executable(parent lab3/parent.c)
add_executable(child lab3/child.c)

target_link_libraries(parent PRIVATE rt pthread)
target_link_libraries(child PRIVATE rt pthread)