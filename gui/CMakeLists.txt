cmake_minimum_required(VERSION 3.10)

project(HXSLGui VERSION 1.0)

add_executable(HXSLGui main.cpp)

target_link_libraries(HXSLGui CompilerStatic)

add_dependencies(HXSLGui copy_example_files)