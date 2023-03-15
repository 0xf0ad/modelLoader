cmake_minimum_required(VERSION 3.25)
project(imgui)

file(GLOB_RECURSE source CONFIGURE_DEPENDS "*.cpp" "backends/*.cpp")

add_library(imgui ${source})
target_include_directories(imgui PUBLIC include/)