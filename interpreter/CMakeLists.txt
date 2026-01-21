cmake_minimum_required(VERSION 3.12)

project(HXSLInterpreter VERSION 1.0 LANGUAGES CXX)

file(GLOB_RECURSE INTERPRETER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(HXSLInterpreter ${INTERPRETER_SOURCES})
target_include_directories(HXSLInterpreter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(HXSLInterpreter CompilerStatic HXSLBackend)



file(GLOB_RECURSE EXAMPLE_MODULE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}example/*.module")

set(DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/modules")

add_custom_command(
    OUTPUT ${DEST_DIR}/dummy.txt 
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/example"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/example" ${DEST_DIR}
    DEPENDS ${EXAMPLE_MODULE_FILES} 
    COMMENT "Copying all .txt files from 'example' to the output directory"
)

add_custom_target(InterpreterCopyModuleFiles ALL DEPENDS ${DEST_DIR}/dummy.txt)
add_dependencies(HXSLInterpreter InterpreterCopyModuleFiles)