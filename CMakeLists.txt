cmake_minimum_required(VERSION 3.27)

#set(CMAKE_SYSTEM_NAME Windows)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")

	set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
	set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
	set(CMAKE_RC_COMPILER  x86_64-w64-mingw32-windres)
	set(CMAKE_RANLIB       x86_64-w64-mingw32-ranlib)

	set(CMAKE_FIND_ROOT_PATH "/usr/x86_64-w64-mingw32")
	set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
	set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
	set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

else()

	set(CMAKE_C_COMPILER   gcc)
	set(CMAKE_CXX_COMPILER g++)

endif()

set(CMAKE_CXX_FLAGS  "-Wall -Wextra -O3")
set(CMAKE_C_FLAGS    "-Wall -Wextra -O3")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

project(modelLoader)

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(src ${OPENGL_INCLUDE_DIRS})

add_subdirectory(headers/libs/stbi)
add_subdirectory(headers/libs/glad)
add_subdirectory(headers/libs/imgui)


# set "cppsource" variables to all .c and .cpp files in "src"
file(GLOB_RECURSE cppsource CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE csource   CONFIGURE_DEPENDS "src/*.c")

add_executable(load ${csource} ${cppsource})
target_link_libraries(load ${OPENGL_LIBRARIES} glad stb_image imgui)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	target_link_libraries(load -lm -lgdi32 -luser32 -lkernel32 -lassimp)
	target_link_libraries(load glfw3)
else()
	target_link_libraries(load -lglfw -lGL -lrt -lm -lX11 -lpthread -lXrandr -lXi -ldl -lxcb -lXau -lXdmcp -lassimp)
endif()

add_custom_command(
	TARGET load
	POST_BUILD
	COMMAND ${CMAKE_COMMAND}
	ARGS -E copy $<TARGET_FILE:load> ${PROJECT_SOURCE_DIR}
)
