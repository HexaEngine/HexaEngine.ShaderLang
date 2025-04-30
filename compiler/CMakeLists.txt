cmake_minimum_required(VERSION 3.10)

project(HXSLCompiler VERSION 1.0)

add_executable(HXSLCompiler main.cpp)

target_link_libraries(HXSLCompiler CompilerStatic)


set(SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/compiler/intrinsics.txt
    ${CMAKE_SOURCE_DIR}/compiler/library.txt
    ${CMAKE_SOURCE_DIR}/compiler/shader.txt
    ${CMAKE_SOURCE_DIR}/compiler/shader2.txt
)


set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/compiler)
file(COPY ${SOURCE_FILES} DESTINATION ${OUTPUT_DIR})