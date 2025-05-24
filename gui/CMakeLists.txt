cmake_minimum_required(VERSION 3.12)
project(HXSLGui VERSION 1.0)

add_executable(HXSLGui main.cpp window_manager.cpp "imgui_window.cpp")

add_dependencies(HXSLGui copy_example_files)

target_link_libraries(HXSLGui CompilerStatic imgui glfw ${CMAKE_DL_LIBS})

find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    target_include_directories(HXSLGui PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(HXSLGui OpenGL::GL)
endif()

