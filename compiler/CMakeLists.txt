cmake_minimum_required(VERSION 3.12)

project(HXSLCompiler VERSION 1.0)

add_executable(HXSLCompiler main.cpp)

target_link_libraries(HXSLCompiler CompilerStatic)

file(GLOB_RECURSE EXAMPLE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}example/*.txt")

set(DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/example")

add_custom_command(
    OUTPUT ${DEST_DIR}/dummy.txt 
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/example"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/example" ${DEST_DIR}
    DEPENDS ${EXAMPLE_FILES} 
    COMMENT "Copying all .txt files from 'example' to the output directory"
)

add_custom_target(copy_example_files ALL DEPENDS ${DEST_DIR}/dummy.txt)

add_dependencies(HXSLCompiler copy_example_files)