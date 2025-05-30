cmake_minimum_required(VERSION 3.12)

project(HXSLBackend VERSION 1.0 LANGUAGES CXX)

file(GLOB_RECURSE BACKEND_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(HXSLBackend STATIC ${BACKEND_SOURCES})

target_compile_definitions(HXSLBackend PRIVATE $<$<CONFIG:Debug>:HXSL_DEBUG=1>)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/fmt ${CMAKE_BINARY_DIR}/external/fmt)

target_link_libraries(HXSLBackend PUBLIC HXSLUtils fmt::fmt)

target_include_directories(HXSLBackend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(HXSLBackend PUBLIC cxx_std_20)
