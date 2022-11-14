cmake_minimum_required(VERSION 3.23)
project(one_shell C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(one_shell
        csapp.c
        csapp.h
        myshell.c)
