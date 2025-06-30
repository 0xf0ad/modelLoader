cmake_minimum_required(VERSION 3.25)
project(imgui)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

file(GLOB_RECURSE source CONFIGURE_DEPENDS "*.cpp" "backends/*.cpp")

add_library(imgui ${source})
target_include_directories(imgui PUBLIC include/)