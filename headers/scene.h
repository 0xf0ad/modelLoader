#if IAMFINISHED // I am not
#pragma once

#include "../headers/libs/imgui/imgui.h"
#include "../headers/libs/imgui/backends/imgui_impl_glfw.h"
#include "../headers/libs/imgui/backends/imgui_impl_opengl3.h"
#include "../headers/shader.h"
#include "../headers/model.h"
#include "../headers/frambuffer.h"
#include "../headers/cubemap.h"
#include "../headers/libs/stbi/stb_image.h"
#include "../headers/camera.h"
#include "../headers/animator.h"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <x86intrin.h>

/* -======settings======- */
int WIN_WIDTH   =                 1280;     // window width
int WIN_HEIGHT  =                  720;     // window height
#define glslVersion "#version 450 core"     // glsl version used by ImGui
#define IWANNASAMPLE               true     // do i want multi sampling anti-aliasing ?
#define MSAA_LVL                      4     // anti-aliassing leve
#define IWANNAADPTHBUFFER          true     // di i want a depth buffer ?
#define IWANNACULLFACES            true     // do i want a face culler ?
#define IWANNABLENDER              true     // do i want to enable blending alpha-values (transparency) ?
#define IWANNATRACKMOUSEPOS        true     // do i want to display mouse positions ?
#define IWANNAUSEAFRAMEBUFFER     false     // do i want to use a frame buffer ?
#define IWANNASTENCILBUFFER        true     // do i want to enable the stancil buffer ?
#define IWANNAGEOMETRYSHADER      false     // do i want a geometry shader ? obviously NO
#define IWANNAASKYBOX              true     // do i want a skybox or a cubemap ?

#define LOG(s)        printf("%s\n", s)     // print any loged "thing" into the stdout (the console)

struct Scene{
	GLFWwindow* window;
	const char* window_name;
	uint32_t    resolution[2];

	Camera*     camera;

	bool        enable_depth;
	bool        enable_culling;
	bool        enable_blending;
	bool        enable_stencilling;
	bool        enable_samplling;
	bool        enable_framebuffer;
	bool        enable_cubemap;
	bool        enable_vsync;

	uint32_t    VAO, VBO, EBO;

	uint64_t    vertices_count;
	uint64_t    indices_count;
	uint32_t    texture_count;

	CubeMap*    skybox;
	Model*      models;
};

inline bool init();
inline void update();


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
#endif