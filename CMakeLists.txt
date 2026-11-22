cmake_minimum_required(VERSION 3.28)
project(os_labs C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

add_compile_options(-Wall -Wextra -pedantic)

add_executable(os_labs lab1/parent.c)
add_executable(child1 lab1/child.c)

add_executable(gaussian lab2/gaussian_threads.c)
target_link_libraries(gaussian PRIVATE pthread m)

add_executable(parent lab3/parent.c)
add_executable(child lab3/child.c)

target_link_libraries(parent PRIVATE rt pthread)
target_link_libraries(child PRIVATE rt pthread)

add_library(impl1 SHARED lab4/impl1.c)
add_library(impl2 SHARED lab4/impl2.c)
target_link_libraries(impl1 PRIVATE m)
target_link_libraries(impl2 PRIVATE m)

add_executable(lab4_prog1 lab4/prog1.c)
target_link_libraries(lab4_prog1 PRIVATE impl1)

add_executable(lab4_prog2 lab4/prog2.c)
target_link_libraries(lab4_prog2 PRIVATE ${CMAKE_DL_LIBS})