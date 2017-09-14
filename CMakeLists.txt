cmake_minimum_required(VERSION 3.1)
project(coroutine)

add_library(coroutine INTERFACE)
target_include_directories(coroutine INTERFACE "${CMAKE_SOURCE_DIR}/include/")

configure_file(
    "${CMAKE_SOURCE_DIR}/.ycm_extra_conf.py.in"
    "${CMAKE_SOURCE_DIR}/.ycm_extra_conf.py"
)
