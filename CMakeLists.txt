cmake_minimum_required(VERSION 3.12)

project(Compiler VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/W3 /Od /Zi)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

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

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

find_package(Python3 REQUIRED)

set(LOCALES_GEN_TOOL "${CMAKE_SOURCE_DIR}/tools/locales_gen.py")
set(GENERATED_HEADER_DIR "${CMAKE_SOURCE_DIR}/src/generated")
set(DIAG_MESSAGE_DIR "${CMAKE_SOURCE_DIR}/resources/diagnostic_messages")
set(LOCALES_OUT_DIR "${CMAKE_BINARY_DIR}/locales")
set(LOCALES_MARKER "${LOCALES_OUT_DIR}/.locales_generated")
set(LOCALIZATION_HPP "${GENERATED_HEADER_DIR}/localization.hpp")

file(GLOB_RECURSE EN_US_FILES CONFIGURE_DEPENDS "${DIAG_MESSAGE_DIR}/en_US/*.md")

add_custom_command(
    OUTPUT "${LOCALIZATION_HPP}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_HEADER_DIR}"
    COMMAND ${Python3_EXECUTABLE} ${LOCALES_GEN_TOOL}
        --locale en_US
        --out_hpp "${LOCALIZATION_HPP}"
    DEPENDS ${LOCALES_GEN_TOOL} ${EN_US_FILES}
    COMMENT "Regenerating localization header..."
    VERBATIM
)

file(GLOB_RECURSE DIAG_FILES CONFIGURE_DEPENDS "${DIAG_MESSAGE_DIR}/*.md")

add_custom_command(
    OUTPUT "${LOCALES_MARKER}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_HEADER_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${LOCALES_OUT_DIR}"
    COMMAND ${Python3_EXECUTABLE} ${LOCALES_GEN_TOOL}
        --locale all
        --out_locales "${LOCALES_OUT_DIR}"
    DEPENDS ${LOCALES_GEN_TOOL} ${DIAG_FILES}
    COMMENT "Generating localization data..."
    VERBATIM
)

add_custom_target(generate_locales ALL DEPENDS "${LOCALES_MARKER}" "${LOCALIZATION_HPP}")
add_definitions(-DHXSL_LOCALE_PATH="${LOCALES_OUT_DIR}")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/zstd/build/cmake)
target_include_directories(zstd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/zstd/lib/)

if (BUILD_SHARED)
    add_library(Compiler SHARED ${SOURCES})

    target_link_libraries(Compiler PRIVATE libzstd_static)

    add_dependencies(Compiler generate_locales)

    target_compile_definitions(Compiler PRIVATE $<$<CONFIG:Debug>:HXSL_DEBUG=1>)
endif()

if (BUILD_STATIC)
    add_library(CompilerStatic STATIC ${SOURCES})

    target_link_libraries(CompilerStatic PRIVATE libzstd_static)

    add_dependencies(CompilerStatic generate_locales)

    target_compile_definitions(CompilerStatic PRIVATE $<$<CONFIG:Debug>:HXSL_DEBUG=1> HXSL_ENABLE_CAPI=1)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/compiler)

if(BUILD_TESTS)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/googletest")
        message(STATUS "GoogleTest submodule is not initialized, initializing now...")
        execute_process(COMMAND git submodule update --init --recursive)
    endif()

    add_subdirectory(external/googletest)

    enable_testing()

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

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
else()
    message(STATUS "GoogleTest is disabled. Skipping tests.")
endif()

