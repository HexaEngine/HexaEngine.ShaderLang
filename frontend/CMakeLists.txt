cmake_minimum_required(VERSION 3.12)

project(HXSLCompiler VERSION 1.0 LANGUAGES CXX)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/zstd/build/cmake ${CMAKE_BINARY_DIR}/external/zstd)
target_include_directories(zstd PUBLIC ${CMAKE_SOURCE_DIR}/external/zstd/lib/)

file(GLOB_RECURSE FRONTEND_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(CompilerCommon INTERFACE)

target_include_directories(CompilerCommon INTERFACE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_definitions(CompilerCommon INTERFACE
    HXSL_AVX2=1
    $<$<CONFIG:Debug>:HXSL_DEBUG=1>
)

target_compile_features(CompilerCommon INTERFACE cxx_std_20)
target_include_directories(CompilerCommon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(CompilerCommon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(CompilerCommon INTERFACE "src/pch/std.hpp" "src/pch/ast.hpp" "src/pch/ast_analyzers.hpp" "src/pch/ast_misc.hpp" "src/pch/localization.hpp")

if (BUILD_SHARED)
    add_library(Compiler SHARED ${FRONTEND_SOURCES})

    target_link_libraries(Compiler PRIVATE libzstd_static)
    target_link_libraries(Compiler PUBLIC CompilerCommon HXSLUtils HXSLBackend)

    add_dependencies(Compiler generate_locales)

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
        target_compile_definitions(Compiler PRIVATE HXSL_X86_64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|AARCH64")
        target_compile_definitions(Compiler PRIVATE HXSL_AARCH64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM")
        target_compile_definitions(Compiler PRIVATE HXSL_ARM)
    else()
        target_compile_definitions(Compiler PRIVATE HXSL_UNKNOWN)
    endif()

    target_compile_definitions(CompilerStatic PRIVATE HXSL_ENABLE_CAPI=1)
endif()

if (BUILD_STATIC)
    add_library(CompilerStatic STATIC ${FRONTEND_SOURCES})

    target_link_libraries(CompilerStatic PRIVATE libzstd_static)
    target_link_libraries(CompilerStatic PUBLIC CompilerCommon HXSLUtils HXSLBackend)

    add_dependencies(CompilerStatic generate_locales)

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
        target_compile_definitions(CompilerStatic PRIVATE HXSL_X86_64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|AARCH64")
        target_compile_definitions(CompilerStatic PRIVATE HXSL_AARCH64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|ARM")
        target_compile_definitions(CompilerStatic PRIVATE HXSL_ARM)
    else()
        target_compile_definitions(CompilerStatic PRIVATE HXSL_UNKNOWN)
    endif()
endif()

