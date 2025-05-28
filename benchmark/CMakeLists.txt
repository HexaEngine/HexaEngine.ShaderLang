cmake_minimum_required(VERSION 3.12)

project(HXSLBenchmark VERSION 1.0)

add_executable(HXSLBenchmark main.cpp)

target_link_libraries(HXSLBenchmark CompilerStatic)

add_dependencies(HXSLBenchmark copy_example_files)