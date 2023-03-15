cmake_minimum_required(VERSION 3.25)


#set(CMAKE_SYSTEM_NAME Windows)

#SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
#SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
#SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
#set(CMAKE_RANLIB i686-w64-mingw32-ranlib)

set(CMAKE_CXX_FLAGS  "-march=native -Wall -Wextra -g -O0")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

project(modelLoader)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/gui)
include_directories(${PROJECT_SOURCE_DIR}/headers)

add_library(ImGui gui/imgui.cpp gui/imgui_demo.cpp gui/imgui_tables.cpp gui/imgui_widgets.cpp gui/imgui_draw.cpp gui/backends/imgui_impl_glfw.cpp gui/backends/imgui_impl_opengl3.cpp)
add_executable(load src/bone.cpp src/cubemap.cpp src/glad.c src/main.cpp src/mesh.cpp src/model.cpp src/shader.cpp src/stb_image.c)
target_link_libraries(load -lglfw -lGL -lrt -lm -lX11 -lpthread -lXrandr -lXi -ldl -lxcb -lXau -lXdmcp -lassimp)
target_link_libraries(load ImGui)

add_custom_command(
	TARGET load
	POST_BUILD
	COMMAND ${CMAKE_COMMAND}
	ARGS -E copy $<TARGET_FILE:load> ${PROJECT_SOURCE_DIR}
)
