cmake_minimum_required(VERSION 3.20)
project(CLion_C_Project_for_CSC173 C)

set(CMAKE_C_STANDARD 99) # 必须写上
set(CMAKE_C_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Werror)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic") # 必须写上



add_executable(bit bits.c)
