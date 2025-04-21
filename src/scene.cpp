#if IAMFINISHED
#include "../headers/scene.h"

Scene* m_scene;


inline bool initwindow(Scene* scene){
	m_scene = scene;
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if(scene->enable_samplling)
		glfwWindowHint(GLFW_SAMPLES, MSAA_LVL);
	#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	//check for GLFW loading errors
	//-----------------------------
	if (!glfwInit()){
		fprintf(stderr, "could not initialize glfw\n");
		return EXIT_FAILURE;
	}

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, scene->window_name, NULL, NULL);
	if (!window){
		fprintf(stderr, "Failed to create GLFW window\n");
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		fprintf(stderr, "Failed to initialize GLAD\n");
		return EXIT_FAILURE;
	}

	// configure global opengl state
	// -----------------------------
	if(scene->enable_depth)
		glEnable(GL_DEPTH_TEST);
	if(scene->enable_culling)
		glEnable(GL_CULL_FACE);
	if(scene->enable_blending){
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	if(scene->enable_stencilling){
		//enable stencil buffer
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	}
	if(scene->enable_samplling)
		glEnable(GL_MULTISAMPLE);

	if(scene->enable_cubemap)
		scene->skybox = new CubeMap;
	else
		scene->skybox = nullptr;

}


inline bool addshaders(Scene* scene, uint8_t number, const char** scripts){
	if (number % 2)
		return true;
	scene->shaders.reserve(number);
	for (uint8_t i = 0; i < number; i += 2)
		scene->shaders.emplace_back(scripts[i], scripts[i+1]);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	m_scene->resolution[0] = height;
	m_scene->resolution[1] = width;
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){

	static double lastX = m_scene->resolution[0] >> 1;
	static double lastY = m_scene->resolution[1] >> 1;
	
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
		m_scene->camera->ProcessMouseMovement(xposIn - lastX, lastY - yposIn);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	lastX = xposIn;
	lastY = yposIn;
}
#endif