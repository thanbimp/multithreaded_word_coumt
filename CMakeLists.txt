cmake_minimum_required(VERSION 3.17)
project(it219148 C)

set(CMAKE_C_STANDARD 11)

add_executable(it219148 main.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")