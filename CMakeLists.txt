cmake_minimum_required(VERSION 3.12)

project(Compiler VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/W3 /Zi /MP)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()


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

find_package(Python3 REQUIRED)

set(LOCALES_GEN_TOOL "${CMAKE_SOURCE_DIR}/tools/locales_gen.py")
set(GENERATED_HEADER_DIR "${CMAKE_SOURCE_DIR}/frontend/src/generated")
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

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/utils)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/backend)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/frontend)

add_subdirectory(${PROJECT_SOURCE_DIR}/compiler)
add_subdirectory(${PROJECT_SOURCE_DIR}/interpreter)
add_subdirectory(${PROJECT_SOURCE_DIR}/gui)
add_subdirectory(${PROJECT_SOURCE_DIR}/benchmark)

if(BUILD_TESTS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
else()
    message(STATUS "GoogleTest is disabled. Skipping tests.")
endif()

