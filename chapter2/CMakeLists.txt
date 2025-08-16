cmake_minimum_required(VERSION 3.31)
project(chapter2 C)

set(CMAKE_C_STANDARD 11)

add_executable(chapter2 main.c
        portsf/ieee80.c
        portsf/portsf.c
)

target_include_directories(chapter2 PRIVATE portsf)