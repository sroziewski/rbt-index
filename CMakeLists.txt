cmake_minimum_required(VERSION 3.30)
project(rbt_index C)

set(CMAKE_C_STANDARD 11)

add_library(rbt_index STATIC library.c)
