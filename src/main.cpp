#include <cstdint>
#include <cstdio>
#include <cassert>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

using b8 = bool;
using b32 = uint32_t;

#define cast static_cast
#define rcast reinterpret_cast
#define null nullptr
#define global static

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb-image.h>

#define FM_IMPLEMENTATION
#include <FastMath.h>
using namespace fm;

#include <irrKlang.h>

constexpr static u32 ScreenWidth = 1920;
constexpr static u32 ScreenHeight = 1080;
static GLFWwindow* Window;

global f32 Dt;
global f32 LastFrame;
global irrklang::ISoundEngine* SoundEngine;

#include "renderUtil.cpp"
#include "game.cpp"

static void 
FrameBufferSizeCallback(GLFWwindow* Window, i32 Width, i32 Height)
{
    glViewport(0, 0, Width, Height);
}

static void 
APIENTRY OpenglErrorCallback(GLenum Source, GLenum Type, GLuint Id, GLenum Severity, GLsizei Length, const GLchar* Message, const void* UserParam)
{
	printf("message: %s\n", Message);
	printf("type:");

	switch(Type) 
	{
		case GL_DEBUG_TYPE_ERROR: {
			puts("ERROR");
		} break;

		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
			puts("DEPRECATED_BEHAVIOR");
		}break;

		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
			puts("UNDEFINED_BEHAVIOR");
		} break;

		case GL_DEBUG_TYPE_PORTABILITY: {
			puts("PORTABILITY");
		} break;

		case GL_DEBUG_TYPE_PERFORMANCE: {
			puts("PERFORMANCE");
		} break;

		case GL_DEBUG_TYPE_OTHER: {
			puts("OTHER");
		} break;
	}

	printf("id: %u", Id);
	puts("severity");
	switch(Severity) 
	{
		case GL_DEBUG_SEVERITY_LOW: {
			puts("LOW");
		} break;

		case GL_DEBUG_SEVERITY_MEDIUM: {
			puts("MEDIUM");
		} break;

		case GL_DEBUG_SEVERITY_HIGH: {
			puts("HIGH");
		} break;
	}

	__debugbreak();
}

i32
main()
{
	// init glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create window
	Window = glfwCreateWindow(ScreenWidth, ScreenHeight, "Learn OpenGL", glfwGetPrimaryMonitor(), null);
    if(!Window)
    {
		puts("Failed to create GLFW window");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(Window);
    glfwSetFramebufferSizeCallback(Window, FrameBufferSizeCallback);

	// init glew
    if(glewInit() != GLEW_OK)
	{
		puts("Failed to init GLEW");
	}

	// init irrklang
	SoundEngine = irrklang::createIrrKlangDevice();
	if(!SoundEngine)
	{
		puts("Failed to start sound engine!");
	}

	// init opengl debug
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(OpenglErrorCallback, null);
	GLuint UnusedIds = 0;
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &UnusedIds, true);

	// init imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(Window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::StyleColorsDark();

	InitApp();	

	// game loop
	while(!glfwWindowShouldClose(Window))
	{
		f32 CurrentFrame = cast<f32>(glfwGetTime());
        Dt = CurrentFrame - LastFrame;
        LastFrame = CurrentFrame;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		UpdateAndRender();	

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapInterval(1);
        glfwSwapBuffers(Window);
        glfwPollEvents();
	}

	SoundEngine->drop();
}

