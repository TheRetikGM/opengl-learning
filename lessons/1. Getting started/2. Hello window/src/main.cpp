#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

// Global Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
// Globals
GLFWwindow* window;

uint8_t init(void);
void framebuffersize_callback(GLFWwindow* window, int width, int height);
void freeAll(void);
void proccessInput(GLFWwindow* window);

int main(int argc, char** argv)
{
	if (init() != 0)
		return -1;

	// MAIN loop
	while (!glfwWindowShouldClose(window)) {
		// input ...
		proccessInput(window);

		// rendering commands here ...
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// check and calls events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	freeAll();
	return 0;
}

uint8_t init(void) {
	// init of glfw window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Window!", NULL, NULL);
	if (window == NULL) {
		cerr << "Failed to create glfw window" << endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	// init of GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cerr << "Failed to initialize GLAD" << endl;
	}

	glfwSetFramebufferSizeCallback(window, framebuffersize_callback);
	return 0;
}
void framebuffersize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
void freeAll(void)
{
	glfwTerminate();
}
void proccessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}