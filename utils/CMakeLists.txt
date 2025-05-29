cmake_minimum_required(VERSION 3.12)

project(HXSLUtils VERSION 1.0 LANGUAGES CXX)

file(GLOB_RECURSE UTILS_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE UTILS_C_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(HXSLUtils STATIC ${UTILS_SOURCES} ${UTILS_C_SOURCES})

target_include_directories(HXSLUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(HXSLUtils PUBLIC cxx_std_20)

if(BUILD_TESTS)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()