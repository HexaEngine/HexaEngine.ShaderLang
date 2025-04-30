cmake_minimum_required(VERSION 3.10)

project(Compiler VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/W3 /Od /Zi)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)

option(BUILD_TESTS "Enable Unit tests for compiler" ON)
option(BUILD_SHARED "Build shared lib" OFF)
option(BUILD_STATIC "Build static lib" ON)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MD>
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
    )
endif()


# Add natvis file to the project if using Visual Studio
if (MSVC)
    set(NATVIS_FILE "${PROJECT_SOURCE_DIR}/visualizers/text_span.natvis")
    if (EXISTS ${NATVIS_FILE})
        set_source_files_properties(${NATVIS_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
        source_group("Visualizers" FILES ${NATVIS_FILE})
        list(APPEND EXTRA_SOURCES ${NATVIS_FILE})
    endif()
endif()

file(GLOB SOURCES
    src/lexical/lexer.cpp
    src/lexical/numbers.cpp
    src/lexical/token_stream.cpp
    src/parsers/parser.cpp
    src/parsers/parser_helper.cpp
    src/parsers/member_path_parser.cpp
    src/parsers/declaration_parser.cpp
    src/parsers/statement_parser.cpp
    src/parsers/expression_parser.cpp
    src/parsers/pratt_parser.cpp
    src/parsers/sub_parser_registry.cpp
    src/symbols/symbol_handle.cpp
    src/symbols/symbol_table.cpp
    src/symbols/symbol_resolver.cpp
    src/symbols/symbol_collector.cpp
    src/symbols/type_checker.cpp
    src/symbols/expression_checker.cpp
    src/symbols/statement_checker.cpp
    src/analyzers/analyzer.cpp
    src/analyzers/sub_analyzer_registry.cpp
    src/analyzers/declaration_analyzer.cpp

    src/utils/object_pool.cpp
    src/utils/text_helper.cpp
    src/utils/allocator.cpp
    src/io/stream.cpp

    src/il/assembly.cpp

    src/config.cpp

    src/ast_modules/ast_base.cpp
    src/ast_modules/container.cpp
    src/ast_modules/symbol_base.cpp
    src/ast_modules/declarations.cpp
    src/ast_modules/statements.cpp
    src/ast_modules/namespace.cpp
    src/ast_modules/array_manager.cpp
    src/ast_modules/primitive_manager.cpp
    src/ast_modules/swizzle_manager.cpp
)

if (BUILD_SHARED)
    add_library(Compiler SHARED ${SOURCES})
endif()

if (BUILD_STATIC)
    add_library(CompilerStatic STATIC ${SOURCES})

    set_target_properties(CompilerStatic PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
    )
endif()


add_subdirectory(${PROJECT_SOURCE_DIR}/compiler)

if(BUILD_TESTS)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/googletest")
        message(STATUS "GoogleTest submodule is not initialized, initializing now...")
        execute_process(COMMAND git submodule update --init --recursive)
    endif()

    add_subdirectory(external/googletest)

    enable_testing()

    add_executable(unit_tests
        tests/pratt_parser_tests.cpp
    )

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
else()
    message(STATUS "GoogleTest is disabled. Skipping tests.")
endif()