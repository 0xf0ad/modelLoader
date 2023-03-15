cmake_minimum_required(VERSION 3.25)


#set(CMAKE_SYSTEM_NAME Windows)

#SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
#SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
#SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
#set(CMAKE_RANLIB i686-w64-mingw32-ranlib)

set(CMAKE_CXX_FLAGS  "-march=native -Wall -Wextra -g -O0")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

project(modelLoader)

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(src ${OPENGL_INCLUDE_DIRS})


add_subdirectory(headers/libs/glad)
add_subdirectory(headers/libs/stbi)
add_subdirectory(headers/libs/imgui)

# set "cppsource" variables to all .c and .cpp files in "src"
file(GLOB_RECURSE cppsource CONFIGURE_DEPENDS "src/*.cpp")
# same here for other files
file(GLOB_RECURSE csource   CONFIGURE_DEPENDS "src/*.c")
#file(GLOB_RECURSE imguisrc  CONFIGURE_DEPENDS "gui/*.cpp")

add_executable(load ${csource} ${cppsource})
target_link_libraries(load ${OPENGL_LIBRARIES} glad stb_image imgui)
target_link_libraries(load -lglfw -lGL -lrt -lm -lX11 -lpthread -lXrandr -lXi -ldl -lxcb -lXau -lXdmcp -lassimp)

add_custom_command(
	TARGET load
	POST_BUILD
	COMMAND ${CMAKE_COMMAND}
	ARGS -E copy $<TARGET_FILE:load> ${PROJECT_SOURCE_DIR}
)
