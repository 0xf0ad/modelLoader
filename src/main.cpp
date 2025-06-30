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

// camera
Camera camera(glm::vec3(0.0f, 3.0f, 9.0f));
double lastX = WIN_WIDTH  >> 1;	// deviding the width by 2 (but divition is expensive)
double lastY = WIN_HEIGHT >> 1;	// insted we will shift the width by 1 witch save us some CPU cycles)

// some useless global variables
// how do I shut those warnnings about initialisation
extern bool outlined = false;
extern bool G_squad = true;

// timing
float deltaTime = 0.0f, lastFrame = 0.0f;

// define functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(Camera* camera,GLFWwindow *window);
static void ShowOverlay(bool* p_open);
static bool ShowCordDialog(bool* p_open, glm::vec3 *AxisRot, float *rotDegre);


int main(int argc, const char** argv){
	bool animated = true;
	bool showOverlay = true;
	bool hasAnimation = true;

	// check for number of arguments
	if(argc == 1){
		fprintf(stderr, "please insert a path to the model you want to view\n");
		return EXIT_FAILURE;
	}else if(argc == 2){
		animated = false;
	}

	const char* animationPath = animated ? argv[2] : argv[1];
	animated = false;

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
		return EXIT_FAILURE;
	}

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, argv[1], NULL, NULL);
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
	Shader ourShader("shaders/main.vert", "shaders/main.frag");
	Shader boneShader("shaders/bone.vert", "shaders/bone.frag");
	Shader outLiner("shaders/outliner.vert", "shaders/outliner.frag");
	#if IWANNAASKYBOX
	Shader skyBoxShader("shaders/skybox.vert", "shaders/skybox.frag");
	#endif

	ubo VP = ubo("VP", 2 * sizeof(glm::mat4));
	VP.bind(0);
	ourShader.bind_ubo(&VP);
	outLiner.bind_ubo(&VP);
	boneShader.bind_ubo(&VP);
	skyBoxShader.bind_ubo(&VP);

	#if IWANNAUSEAFRAMEBUFFER
		// define a framebuffer object
		FrameBuffer framebuffer(WIN_WIDTH, WIN_HEIGHT);
	#endif

	#if IWANNAASKYBOX
		CubeMap cubemap;
	#endif

	// load models
	// -----------

	Model ourModel(argv[1]);
	Model bonemodel("assets/bone.obj");

	LOG("model got loaded");
	cyclesDiffrence = finishCylceID - initCycleID;

	Animation *animation;
	Animator  *animator;

	ourShader.use();
	ourShader.setBool("animated", animated);
	ourShader.setFloat("modelsize", 1.0f);
	boneShader.use();
	boneShader.setBool("animated", animated);
	boneShader.setFloat("modelsize", 1.0f);
	outLiner.use();
	outLiner.setBool ("animated", animated);


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
	float scale              = 0.0f;
	ImVec4 clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);

	// disable V-Sync to get more than 60 fps
	glfwSwapInterval(V_Sync);
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 projection = glm::perspective(camera.mFieldOfView, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.getViewMatrix();
	model = glm::scale(model, glm::vec3(0.25));
	VP.editBuffer(0, sizeof(glm::mat4), &projection);
	VP.editBuffer(sizeof(glm::mat4), sizeof(glm::mat4), &view);
	glm::vec3 AxisRot = glm::vec3(0.0f);
	float rotDegre = 0.0f;
	float modelsize = 1.0f;
	float animespeed = 1.0f;
	float animationplay = 0.0f; // its a boolean but its better be float to easly multiply (i know this doesnt make sense but trust me I am an engineer)
	bool mooving = false;
	bool renderbones = false;
	int animIndex = 0;
	char uniform[] = "b_Mats[   ]"; // bone matrices

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window)){
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		//deltaTime = 1.0f / ImGui::GetIO().Framerate;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		/*if(msaaLvl != prevMsaaLvl){
			glfwTerminate();
			glfwInit();
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_SAMPLES, 1);
			printf("MSAAA GO BRRRR\n");
			
		}
		prevMsaaLvl = msaaLvl;*/

		// input
		// -----
		processInput(&camera, window);

		// render
		// ------
		#if IWANNAUSEAFRAMEBUFFER
			framebuffer.firstPass();
		#endif
		glEnable(GL_DEPTH_TEST);
		float *clrColorPtr = (float*)&clear_color;
		//glClearColor(*clrColorPtr, *(clrColorPtr+1), *(clrColorPtr+2), *(clrColorPtr+3));
		// those commented implementation do the same think
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
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

		if (ImGui::SliderFloat("field of view", &camera.mFieldOfView, 0.0f, PI)){
			projection = glm::perspective(camera.mFieldOfView, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 100.0f);
			VP.editBuffer(0, sizeof(glm::mat4), &projection);
		}
		ImGui::SliderFloat("Camera Speed", &camera.maxSpeed, 0.0f, 50.0f);
		if(ImGui::SliderFloat("model size (inverslly)", &modelsize, 0.0f, 20.0f)){
			ourShader.use();
			ourShader.setFloat("modelsize", modelsize);
			if (renderbones){
				boneShader.use();
				boneShader.setFloat("modelsize", modelsize);
			}
		}
		ImGui::SliderFloat("Mouse Sensitivity",&camera.mMouseSensitivity, 0.0f, 1.0f);

		// controlling face rendering (render only front faces or only back ones)
		ImGui::Checkbox("mapping uniforms", &ourShader.mapped);
		ImGui::Checkbox("render front and back faces", &cullFace);

		#if IWANNACULLFACES
			ImGui::SameLine();
			if (ImGui::Button("apply face culling"))
				cullFace ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
		#endif

		ImGui::Checkbox("squad", &G_squad);

		// controlling wireframe mode
		// --------------------------
		ImGui::Checkbox("Render on wireframe", &wireFrame);
		ImGui::SameLine();
		if (ImGui::Button("apply wireframe"))
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL - wireFrame);


		// controlling V-Sync (fix framerate to display refreshrate(likely 60 Hz))
		// -----------------------------------------------------------------------
		ImGui::Checkbox("V-Sync", &V_Sync);
		ImGui::SameLine();
		if (ImGui::Button("apply V-Sync"))
			glfwSwapInterval(V_Sync);

		if(ourModel.hasAnimation){
			if(ImGui::Checkbox("animate", &animated)){
				if(animated){
					animation = new Animation(animationPath, &ourModel);
					LOG("new animation got allocatedhh");
					animator  = new Animator(animation);
					animator->PlayAnimation(animation);
					animator->mCurrentTime=0.0f;
					LOG("animator got createdhh");
				} else {
					delete animation;
					delete animator;
				}
				ourShader.use();
				ourShader.setBool("animated", animated);
				boneShader.use();
				boneShader.setBool("animated", animated);
				outLiner.use();
				outLiner.setBool ("animated", animated);
			}
		}


		if(animated){
			if(ImGui::Combo("animations", &animIndex, animation->mAnimationsNames, animation->mNumAnimations)){
				delete animation;
				LOG("animation got deleted");
				animation = new Animation(animationPath, &ourModel, animIndex);
				LOG("new animation got allocated");
				animator->PlayAnimation(animation);
				LOG("new animation is now playing");
			}

			ImGui::Checkbox("show bones", &renderbones);

			ImGui::SliderFloat("animation speed", &animespeed, -10.0f, 10.0f);
			const char* astring = animationplay ? "pause" : "play";
			if(ImGui::Button(astring))
				animationplay = animationplay == 1.0f ? 0.0f : 1.0f;
		}

		mooving = ShowCordDialog(&show_cordSet_window, &AxisRot, &rotDegre);

		// displaying text or some variable for debugging for noow its disabled
		#if false
		ImGui::Text("Application average %f ms/frame (FPS)", (1.0f / ImGui::GetIO().Framerate) - deltaTime);
		#endif
		ImGui::End();

		ShowOverlay(&showOverlay);

		// render the loaded model by setting the model transformation
		view = camera.getViewMatrix();
		VP.editBuffer(sizeof(glm::mat4), sizeof(glm::mat4), &view);
		// enable shader before setting uniforms
		ourShader.use();
		// seting uniforms
		ourShader.setMat4("model", model);
		if (renderbones){
			boneShader.use();
			boneShader.setMat4("model", model);
		}

		if(mooving)
			model = glm::rotate(model, rotDegre, AxisRot);

		#if IWANNAASKYBOX
		skyBoxShader.use();
		cubemap.drawCubeMap();
		#endif

		if (animated){
			animator->UpdateAnimation(deltaTime, animespeed * animationplay);
			ourShader.use();
			for (uint8_t i = 0; i != animator->boneNumber; i++){
				sprintf(uniform, "b_Mats[%u]", i);
				ourShader.setMat4(uniform, animator->mFinalBoneMatrices[i]);
			}
			if (renderbones){
				boneShader.use();
				for (uint8_t i = 1; i != animator->boneNumber; i++){
					sprintf(uniform, "p_Mats[%u]", i);
					boneShader.setMat4(uniform, animator->mParentBoneMatrices[i]);
				}
			}
		}

		#if IWANNASTENCILBUFFER
		// write to stencil buffer
		glStencilFunc(GL_ALWAYS, true, 0xFF);
		glStencilMask(0xFF);
		#endif

		// draw our model
		ourShader.use();
		ourModel.Draw(ourShader);
		if (renderbones){
			glDisable(GL_DEPTH_TEST);
			boneShader.use();
			bonemodel.Draw(boneShader, ourModel.m_BoneCounter - 1);
			glEnable(GL_DEPTH_TEST);
		}

		if(outlined){
			glDisable(GL_DEPTH_TEST);
			glStencilFunc(GL_NOTEQUAL, true, 0xFF);
			glStencilMask(0x00);
			outLiner.use();
			outLiner.setFloat("scale", scale);
			outLiner.setMat4 ("model", model);
			ourModel.Draw(outLiner);
			if (animated){
				outLiner.use();
				for (uint8_t i = 0; i != animator->boneNumber; i++){
					sprintf(uniform, "b_Mats[%u]", i);
					outLiner.setMat4(uniform, animator->mFinalBoneMatrices[i]);
				}
			}
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
void processInput(Camera* camera,GLFWwindow *window){
	// terminate the process when pressing ESCAPE on the keyboard
	// and move the camera around with WASD keys
	// and speed up or slow down the camera using left shift
	// for now space and controll are buggy
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	//if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){}
	//if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){}
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->ProcessKeyboard(FORWARD, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->ProcessKeyboard(LEFT, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->ProcessKeyboard(BACKWARD, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->ProcessKeyboard(RIGHT, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera->ProcessKeyboard(SLOW, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		camera->ProcessKeyboard(FAST, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera->ProcessKeyboard(GO_UP, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera->ProcessKeyboard(GO_DOWN, deltaTime);
	if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
		//msaaLvl = 8;
	}
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

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
		camera.ProcessMouseMovement(xposIn - lastX, lastY - yposIn);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	lastX = xposIn;
	lastY = yposIn;
}

static void ShowOverlay(bool* p_open){
	static uint8_t corner = 0;
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

		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
		else
			ImGui::Text("Mouse Position: <invalid>");

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

static bool ShowCordDialog(bool* p_open, glm::vec3 *AxisRot, float *rotDegre){
	// corect cordinates if loaded incorrectly
	// ---------------------------------------
	bool willMove = false;
	if(ImGui::Button("cordinates System incorrect ?"))
		*p_open = true;

	if (*p_open){
		ImGui::Begin("correct cordinate system", p_open);
		ImGui::Text("how we can correct your model's cordinates ?");
		if (ImGui::Button("Rotate by  90° on X axis")){
			*AxisRot = glm::vec3(1.0f, 0.0f, 0.0f);
			*rotDegre = HALF_PI; // 90 degrees
			willMove = true;
		}
		else if (ImGui::Button("Rotate by -90° on X axis")){
			*AxisRot = glm::vec3(1.0f, 0.0f, 0.0f);
			*rotDegre = -HALF_PI; // -90 dgrees
			willMove = true;
		}
		else if (ImGui::Button("Rotate by  90° on Y axis")){
			*AxisRot = glm::vec3(0.0f, 1.0f, 0.0f);
			*rotDegre = HALF_PI; // 90 degrees
			willMove = true;
		}
		else if (ImGui::Button("Rotate by -90° on Y axis")){
			*AxisRot = glm::vec3(0.0f, 1.0f, 0.0f);
			*rotDegre = -HALF_PI; // -90 degrees
			willMove = true;
		}
		else if (ImGui::Button("Rotate by  90° on Z axis")){
			*AxisRot = glm::vec3(0.0f, 0.0f, 1.0f);
			*rotDegre = HALF_PI; // 90 degrees
			willMove = true;
		}
		else if (ImGui::Button("Rotate by -90° on Z axis")){
			*AxisRot = glm::vec3(0.0f, 0.0f, 1.0f);
			*rotDegre = -HALF_PI; // -90 degrees
			willMove = true;
		}
		else if (ImGui::Button("Close")){
			*p_open = false;
		}
		ImGui::End();
	}
	return willMove;
}
