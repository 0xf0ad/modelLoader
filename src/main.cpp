#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "../headers/shader.h"
#include "../headers/model.h"
#include "../headers/libs/stb_image.h"
#include "../headers/camera.h"
#include "../headers/animator.h"
#include <GLFW/glfw3.h>
#include <stdio.h>

// settings
#define WIN_WIDTH                  1280
#define WIN_HEIGHT                  720
#define FOV                       45.0f
#define glslVersion "#version 330 core"

// camera
Camera camera(glm::vec3(0.0f, 3.0f, 9.0f));
float lastX = WIN_WIDTH  >> 1;	// deviding the width by 2 (but divition is expensive 
float lastY = WIN_HEIGHT >> 1;	// insted we will shift the width by 1 witch save us some CPU cycles)

bool firstMouse = true;
bool showOverlay = true;
bool animated = true;

// timing
float deltaTime, lastFrame;

// define functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
static void ShowOverlay(bool* p_open);


int main(int argc, char** argv){

	//check for number of arguments
	if(argc == 1){
		printf("please insert a path to the model you want to view\n");
		return true;
	}

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//check for GLFW loading errors
	//-----------------------------
	if (!glfwInit()){
		printf("could not initialize glfw\n");
		return -1;
	}

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "wa7d_rajl_l9ito_f_QUAKE", NULL, NULL);
	if (!window){
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_STENCIL_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("shaders/vertexShader", "shaders/fragmentShader");

	// load models
	// -----------
	Model ourModel(argv[1]);

	Animation *animation;
	Animator  *animator;

	if (animated){
		animation = new Animation(argv[2], &ourModel);
		animator  = new Animator(animation);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void) io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);

	bool show_demo_window    = false;
	bool show_another_window = false;
	bool show_cordSet_window = false;
	bool cullFace            = false;
	bool wireFrame           = false;
	bool V_Sync              = false;
	float f                  = 0.0f;
	ImVec4 clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);

	// disable V-Sync to get more than 60 fps
	glfwSwapInterval(V_Sync);
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 projection;
	glm::mat4 view;

	
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)){
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		//deltaTime = 1.0f / ImGui::GetIO().Framerate;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		if (animated){
			animator->UpdateAnimation(deltaTime);
			for (unsigned int i = 0; i < 100; i++)
				ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + ']', animator->m_FinalBoneMatrices[i]);
		}

		// render
		// ------
		float *clrColorPtr = (float*)&clear_color;
		glClearColor(*clrColorPtr, *(clrColorPtr+1), *(clrColorPtr+2), *(clrColorPtr+3));
		//those commented implementation do the same think
		//glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		//glClearColor(*(float*)&clear_color, *((float*)&clear_color+1), *((float*)&clear_color+2), *((float*)&clear_color+3));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.

		ImGui::Begin("some settings");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is a text.");                         // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", clrColorPtr);          // Edit 3 floats representing a color

		//if (ImGui::Button("Botton")) { }                      // Buttons return true when clicked (most widgets return true when edited/activated)

		// controlling face rendering (render only front faces or only back ones)
		// ----------------------------------------------------------------------
		ImGui::Checkbox("render back faces", &cullFace);
		if (cullFace)
			glCullFace(GL_FRONT);
		else
			glCullFace(GL_BACK);

		// controlling wireframe mode
		// --------------------------
		ImGui::Checkbox("Render on wireframe", &wireFrame);
		if (wireFrame)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// controlling V-Sync (fix framerate to display refreshrate(likely 60 Hz))
		// -----------------------------------------------------------------------
		ImGui::Checkbox("V-Sync", &V_Sync);
		if (ImGui::Button("apply"))
			glfwSwapInterval(V_Sync);

		// corect cordinates if loaded incorrectly
		// ---------------------------------------
		ImGui::Checkbox("cordinate System incorrect ?", &show_cordSet_window);
		if (show_cordSet_window){
			ImGui::Begin("correct cordinate system", &show_cordSet_window);
			ImGui::Text("how we can correct your model's cordinates ?");
			if (ImGui::Button("Rotate by  90° on X axis")){}
			if (ImGui::Button("Rotate by -90° on X axis")){}
			if (ImGui::Button("Rotate by  90° on Y axis")){}
			if (ImGui::Button("Rotate by -90° on Y axis")){}
			if (ImGui::Button("Rotate by  90° on Z axis")){}
			if (ImGui::Button("Rotate by -90° on Z axis")){}
			if (ImGui::Button("Close"))
				show_cordSet_window = false;
			ImGui::End();
		}



		// displaying text or some variable for debugging for noow its disabled
		#if false
		ImGui::Text("Application average %f ms/frame (FPS)", (1.0f / ImGui::GetIO().Framerate) - deltaTime);
		#endif
		ImGui::End();

		// 3. Show another simple window.
		if (show_another_window){
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		ShowOverlay(&showOverlay);

		//enable shader before setting uniforms
		ourShader.use();
		ourShader.setBool("animated", animated);

		// view/projection transformations
		projection = glm::perspective(glm::radians(camera.Zoom), (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		// render the loaded model by setting the model transformation
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
		ourShader.setMat4("model", model);

		// write to stencil buffer
		glStencilMask(0x00);

		// draw our model
		ourModel.Draw(ourShader);

		// tell ImGui to render the windows
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	if (animated){
		// free animation data from memory
		delete animation;
		delete animator;
	}

	//terminate ImGui processs after quitting the loop
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------
	glfwTerminate();
	return false;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window){
	//terminate the process when pressing ESCAPE on the keyboard
	//and switch to wireframe mode when pressing Z key an switch back by pressing X
	//and transform the camera with WASD keys
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) { }
	if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) { }
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(SLOW, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		camera.ProcessKeyboard(NSLOW, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(GO_UP, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.ProcessKeyboard(GO_DOWN, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
	if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
	if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS){}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn){

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if(firstMouse){
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;
	
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
		camera.ProcessMouseMovement(xoffset, yoffset);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE){
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}


static void ShowOverlay(bool* p_open){
	static unsigned char corner = 0;
	ImGuiIO& io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	const float PAD = 10.0f;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
	ImVec2 work_size = viewport->WorkSize;
	ImVec2 window_pos, window_pos_pivot;
	window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
	window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
	window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
	window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
	window_flags |= ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

	if (ImGui::Begin("Simple Overlay", p_open, window_flags)){
		ImGui::Text("debugging overlay\n" "in the corner of the screen.\n" "(right-click to change position)");
		ImGui::Separator();

		if (ImGui::IsMousePosValid()){
			ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
		}else{
			ImGui::Text("Mouse Position: <invalid>");}

		ImGui::Text("App average %.3f ms/frame\n framerate : (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		if (ImGui::BeginPopupContextWindow()){
			if (ImGui::MenuItem("Top-left",     NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right",    NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left",  NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}
