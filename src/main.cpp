#include "../gui/imgui.h"
#include "../gui/backends/imgui_impl_glfw.h"
#include "../gui/backends/imgui_impl_opengl3.h"
#include "../headers/shader.h"
#include "../headers/model.h"
#include "../headers/frambuffer.h"
#include "../headers/cubemap.h"
#include "../headers/libs/stb_image.h"
#include "../headers/camera.h"
#include "../headers/animator.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <x86intrin.h>

/* -======settings======- */
#define WIN_WIDTH                  1280		// window width
#define WIN_HEIGHT                  720		// window height
#define glslVersion "#version 450 core"		// glsl version used by ImGui
#define IWANNASAMPLE               true		// do i want multi sampling anti-aliasing ?
#define MSAA_LVL                      4		// anti-aliassing leve
#define IWANNAADPTHBUFFER          true		// di i want a depth buffer ?
#define IWANNACULLFACES            true		// do i want a face culler ?
#define IWANNABLENDER              true		// do i want to enable blending alpha-values (transparency) ?
#define IWANNATRACKMOUSEPOS       false		// do i want to display mouse positions ?
#define IWANNAUSEAFRAMEBUFFER     false		// do i want to use a frame buffer ?
#define IWANNASTENCILBUFFER        true		// do i want to enable the stancil buffer ?
#define IWANNAGEOMETRYSHADER      false		// do i want a geometry shader ? obviously NO
#define IWANNAASKYBOX              true		// do i want a skybox or a cubemap ?

#define LOG(s)        printf("%s\n", s)		// print any loged "thing" into the stdout (the console)

// camera
Camera camera(glm::vec3(0.0f, 3.0f, 9.0f));
float lastX = WIN_WIDTH  >> 1;	// deviding the width by 2 (but divition is expensive 
float lastY = WIN_HEIGHT >> 1;	// insted we will shift the width by 1 witch save us some CPU cycles)

bool firstMouse = true;
bool showOverlay = true;
bool animated = true;

// some useless global variables
// how do I shut those warnnings about initialisation
extern bool outlined = false;
extern bool Q_squad = true;

// timing
float deltaTime = 0.0f, lastFrame = 0.0f;

// define functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
static void ShowOverlay(bool* p_open);
static void ShowCordDialog(bool* p_open, glm::vec3 *AxisRot, float *rotDegre);


int main(int argc, char** argv){

	// check for number of arguments
	if(argc == 1){
		fprintf(stderr, "please insert a path to the model you want to view\n");
		return -1;
	}else if(argc == 2){
		animated = false;
	}


	unsigned long long initCycleID, finishCylceID, cyclesDiffrence;

	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, MSAA_LVL);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	//check for GLFW loading errors
	//-----------------------------
	if (!glfwInit()){
		fprintf(stderr, "could not initialize glfw\n");
		return -1;
	}

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "wa7d_rajl_l9ito_f_QUAKE", NULL, NULL);
	if (!window){
		fprintf(stderr, "Failed to create GLFW window\n");
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
		fprintf(stderr, "Failed to initialize GLAD\n");
		return -1;
	}

	// configure global opengl state
	// -----------------------------

	#if IWANNAADPTHBUFFER
		// enable depth buffer 
		glEnable(GL_DEPTH_TEST);
	#endif

	#if IWANNACULLFACES
		// enable face culling 
		glEnable(GL_CULL_FACE);
	#endif

	#if IWANNABLENDER
		// enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	#endif

	#if IWANNASTENCILBUFFER
		//enable stencil buffer
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	#endif

	#if IWANNASAMPLE
		// enable multisampling anti-aliasing
		glEnable(GL_MULTISAMPLE);
	#endif


	// build and compile shaders
	// -------------------------
	initCycleID = __rdtsc();
	Shader ourShader("shaders/vertexShader", "shaders/fragmentShader");
	finishCylceID = __rdtsc();
	cyclesDiffrence = finishCylceID - initCycleID;

	printf("%llu\t ticks on shader 1\n", cyclesDiffrence);

	initCycleID = __rdtsc();
	Shader outLiner ("shaders/outlinervs"  , "shaders/outlinerfs"    );
	finishCylceID = __rdtsc();
	cyclesDiffrence = finishCylceID - initCycleID;

	printf("%llu\t ticks on shader 2\n", cyclesDiffrence);

	initCycleID = __rdtsc();
	#if IWANNAASKYBOX
		Shader skyBoxShader("shaders/skyboxvs" ,"shaders/skyboxfs"       );
	#endif
	finishCylceID = __rdtsc();
	cyclesDiffrence = finishCylceID - initCycleID;

	printf("%llu\t ticks on shader 3\n", cyclesDiffrence);

	unsigned int uniformBufferBlock;
	glGenBuffers(1, &uniformBufferBlock);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferBlock);
	glBufferData(GL_UNIFORM_BUFFER, 152, NULL, GL_STATIC_DRAW); // 152 bytes
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	#if IWANNAUSEAFRAMEBUFFER
		// define a framebuffer object
		FrameBuffer framebuffer(WIN_WIDTH, WIN_HEIGHT);
	#endif

	#if IWANNAASKYBOX
		initCubeMap();
	#endif

	// load models
	// -----------

	initCycleID = __rdtsc();

	Model ourModel(argv[1]);
	
	finishCylceID = __rdtsc();
	LOG("model got loaded");
	cyclesDiffrence = finishCylceID - initCycleID;

	printf("%llu\t ticks on loading model\n", cyclesDiffrence);

	Animation *animation;
	Animator  *animator;

	/*if(ourModel.loadAnim()){
		animation = (Animation*) ourModel.loadAnim();
		LOG("animation got loaded from model");
		animator  = new Animator(animation);
		LOG("animator got created from model");
	}else*/ if (animated){
		animation = new Animation(argv[2], &ourModel);
		LOG("animation got loaded");
		animator  = new Animator(animation);
		LOG("animator got created");
	}

	ourShader.use();
	ourShader.setBool("animated", animated);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void) io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);

	bool show_demo_window    = false;
	bool show_cordSet_window = false;
	bool cullFace            = true;
	bool wireFrame           = false;
	bool V_Sync              = false;
	//float f                = 0.0f;
	float scale              = 0.0f;
	ImVec4 clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);

	// disable V-Sync to get more than 60 fps
	glfwSwapInterval(V_Sync);
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 AxisRot = glm::vec3(0.0f);
	float rotDegre = 0.0f;
	int animIndex = 0;


	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)){
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		//deltaTime = 1.0f / ImGui::GetIO().Framerate;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		#if IWANNAUSEAFRAMEBUFFER
			framebuffer.firstPass();
		#endif
		glEnable(GL_DEPTH_TEST);
		float *clrColorPtr = (float*)&clear_color;
		glClearColor(*clrColorPtr, *(clrColorPtr+1), *(clrColorPtr+2), *(clrColorPtr+3));
		// those commented implementation do the same think
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
		ImGui::Begin("some settings");                              // Create a window called "some settings" and append into it.

		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("outlining", &outlined);                // check box for outlinig a model

		if(outlined) ImGui::SliderFloat("outline scale", &scale, 0.0f, 2.0f);
		ImGui::ColorEdit3("clear color", clrColorPtr);        // Edit 3 floats representing a the background color

		ImGui::SliderFloat("field of view", &camera.Zoom, 0.0f, 180.0f);
		//if (ImGui::Button("Botton")) { }                                 // Buttons return true when clicked

		// controlling face rendering (render only front faces or only back ones)
		ImGui::Checkbox("mapping uniforms", &ourShader.mapped);
		ImGui::Checkbox("render front and back faces", &cullFace);

		#if IWANNACULLFACES
			ImGui::SameLine();
			if (ImGui::Button("apply face culling")){
				if (cullFace)
					glEnable(GL_CULL_FACE);
				else
					glDisable(GL_CULL_FACE);
			}
		#endif

		ImGui::Checkbox("squad", &Q_squad);

		// controlling wireframe mode
		// --------------------------
		ImGui::Checkbox("Render on wireframe", &wireFrame);
		ImGui::SameLine();
		if (ImGui::Button("apply wireframe")){
			if (wireFrame)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// controlling V-Sync (fix framerate to display refreshrate(likely 60 Hz))
		// -----------------------------------------------------------------------
		ImGui::Checkbox("V-Sync", &V_Sync);
		ImGui::SameLine();
		if (ImGui::Button("apply V-Sync"))
			glfwSwapInterval(V_Sync);


		if(animated){
			if(ImGui::Combo("animations", &animIndex, animation->m_AnimationsNames.data(), animation->m_AnimationsNames.size())){
				delete animation;
				LOG("animation got deleted");
				animation = new Animation(argv[2], &ourModel, animIndex);
				LOG("new animation got allocated");
				animator->PlayAnimation(animation);
				LOG("animator got updated to the new animation");
			}
		}


		//float line_vertex[]={ 10,10,10, 20,20,20 };

		//glVertexPointer(2, GL_FLOAT, 0, line_vertex);
		//glDrawArrays(GL_LINES, 0, 2);

		/*glBegin(GL_LINES);
			glColor3f (0.0f, 0.0f, 0.0f);
			glVertex3f(10, 10, 10);
			glVertex3f(20, 20, 20);
		glEnd();*/




		ShowCordDialog(&show_cordSet_window, &AxisRot, &rotDegre);

		// displaying text or some variable for debugging for noow its disabled
		#if false
		ImGui::Text("Application average %f ms/frame (FPS)", (1.0f / ImGui::GetIO().Framerate) - deltaTime);
		#endif
		ImGui::End();

		ShowOverlay(&showOverlay);



		// view/projection transformations
		projection = glm::perspective(glm::radians(camera.Zoom), (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		// render the loaded model by setting the model transformation
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));  // it's a bit too big for our scene, so scale it down
		if(rotDegre != 0.0f){
			model = glm::rotate(model, rotDegre, AxisRot);
		}

		#if IWANNAASKYBOX
			skyBoxShader.use();
			skyBoxShader.setMat4("view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
			skyBoxShader.setMat4("projection", projection);
			drawCubeMap();
		#endif



		// enable shader before setting uniforms
		ourShader.use();
		// seting uniforms
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view"      , view);
		ourShader.setMat4("model"     , model);
		ourShader.setVec3("cameraPos" , camera.Position);

		if (animated){
			animator->UpdateAnimation(deltaTime);
			for (unsigned int i = 0; i != 256; i++)
				ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + ']', animator->m_FinalBoneMatrices[i]);
		}

		#if IWANNASTENCILBUFFER
			// write to stencil buffer
			glStencilFunc(GL_ALWAYS, true, 0xFF);
			glStencilMask(0xFF);
		#endif

		// draw our model
		ourModel.Draw(ourShader);

		if(outlined){
			glDisable(GL_DEPTH_TEST);
			glStencilFunc(GL_NOTEQUAL, true, 0xFF);
			glStencilMask(0x00);
			outLiner.use();
			outLiner.setFloat("scale", scale);
			outLiner.setMat4 ("projection", projection);
			outLiner.setMat4 ("view", view);
			outLiner.setMat4 ("model", model);
			outLiner.setBool ("animated", animated);
			ourModel.Draw(outLiner);
			if (animated)
				for (unsigned int i = 0; i != 100; i++)
					outLiner.setMat4("finalBonesMatrices[" + std::to_string(i) + ']', animator->m_FinalBoneMatrices[i]);
			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, false, 0xFF);
			glEnable(GL_DEPTH_TEST);
		}

		#if IWANNAUSEAFRAMEBUFFER
			framebuffer.secondPass();
		#endif

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
		LOG("animation got deleted");
		delete animator;
		LOG("animator got deleted");
	}

	//terminate ImGui processs after quitting the loop
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	LOG("ImGui frees resources");

	// terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------
	#if IWANNAUSEAFRAMEBUFFER
		framebuffer.clear();
	#endif
	glfwTerminate();
	return EXIT_SUCCESS;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window){
	// terminate the process when pressing ESCAPE on the keyboard
	// and move the camera around with WASD keys
	// and speed up or slow down the camera using left shift
	// for now space and controll are buggy
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	//if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){}
	//if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){}
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
	//if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){}
	//if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){}
	//if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS){}
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
		#if IWANNATRACKMOUSEPOS

		if (ImGui::IsMousePosValid()){
			ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
		}else{
			ImGui::Text("Mouse Position: <invalid>");}

		#endif /* IWANNATRACKMOUSEPOS */
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

static void ShowCordDialog(bool* p_open, glm::vec3 *AxisRot, float *rotDegre){
	// corect cordinates if loaded incorrectly
	// ---------------------------------------
	//ImGui::Checkbox("cordinate System incorrect ?", p_open);
	if(ImGui::Button("cordinates System incorrect ?"))
		*p_open = true;

	if (*p_open){
		ImGui::Begin("correct cordinate system", p_open);
		ImGui::Text("how we can correct your model's cordinates ?");
		if (ImGui::Button("Rotate by  90° on X axis")){
			*AxisRot = glm::vec3(1.0f, 0.0f, 0.0f);
			*rotDegre = glm::radians(90.0f);
		}
		else if (ImGui::Button("Rotate by -90° on X axis")){
			*AxisRot = glm::vec3(1.0f, 0.0f, 0.0f);
			*rotDegre = glm::radians(-90.0f);
		}
		else if (ImGui::Button("Rotate by  90° on Y axis")){
			*AxisRot = glm::vec3(0.0f, 1.0f, 0.0f);
			*rotDegre = glm::radians(90.0f);
		}
		else if (ImGui::Button("Rotate by -90° on Y axis")){
			*AxisRot = glm::vec3(0.0f, 1.0f, 0.0f);
			*rotDegre = glm::radians(-90.0f);
		}
		else if (ImGui::Button("Rotate by  90° on Z axis")){
			*AxisRot = glm::vec3(0.0f, 0.0f, 1.0f);
			*rotDegre = glm::radians(90.0f);
		}
		else if (ImGui::Button("Rotate by -90° on Z axis")){
			*AxisRot = glm::vec3(0.0f, 0.0f, 1.0f);
			*rotDegre = glm::radians(-90.0f);
		}
		else if (ImGui::Button("Reset")){
			*rotDegre = 0.0f;
		}
		else if (/*ImGui::SameLine();*/ ImGui::Button("Close"))
			*p_open = false;
		ImGui::End();
	}
}
