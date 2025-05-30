cmake_minimum_required(VERSION 3.12)

project(HXSLCompiler VERSION 1.0 LANGUAGES CXX)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/googletest")
    message(STATUS "GoogleTest submodule is not initialized, initializing now...")
    execute_process(COMMAND git submodule update --init --recursive)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/external/googletest ${CMAKE_BINARY_DIR}/external/googletest)

enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(unit_tests ${TEST_SOURCES})

target_link_libraries(unit_tests
    PRIVATE
        gtest
        gtest_main
        CompilerStatic
)

set_target_properties(unit_tests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

file(GLOB TXT_FILES ${CMAKE_SOURCE_DIR}/tests/pratt_parser_tests/*.txt)

if(TXT_FILES)
    file(COPY ${TXT_FILES} DESTINATION ${CMAKE_BINARY_DIR}/pratt_parser_tests)
else()
    message(WARNING "No .txt files found in ${CMAKE_SOURCE_DIR}/tests/pratt_parser_tests/")
endif()

include(GoogleTest)
gtest_discover_tests(unit_tests)
