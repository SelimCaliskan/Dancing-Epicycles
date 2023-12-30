cmake_minimum_required(VERSION 3.10.0)

project(dancing_circles)
add_executable(dancing_circles "main.c" "include/raygui.h")

target_include_directories(dancing_circles PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(dancing_circles "${PROJECT_SOURCE_DIR}/lib/raylib.lib" winmm.lib)
