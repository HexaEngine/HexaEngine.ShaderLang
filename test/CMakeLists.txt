cmake_minimum_required(VERSION 3.10)

project(TestApp VERSION 1.0)

add_executable(TestApp main.cpp)

target_link_libraries(TestApp CompilerStatic)


set(SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/test/intrinsics.txt
    ${CMAKE_SOURCE_DIR}/test/library.txt
    ${CMAKE_SOURCE_DIR}/test/shader.txt
    ${CMAKE_SOURCE_DIR}/test/shader2.txt
)


set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/test)
file(COPY ${SOURCE_FILES} DESTINATION ${OUTPUT_DIR})