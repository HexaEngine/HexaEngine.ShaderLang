cmake_minimum_required(VERSION 3.10)

project(IntrinsicGen VERSION 1.0)

add_executable(IntrinsicGen main.cpp)

set(SOURCE_FILE ${CMAKE_SOURCE_DIR}/tools/def.txt)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/tools)

add_custom_command(TARGET IntrinsicGen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE} ${OUTPUT_DIR}/def.txt
)