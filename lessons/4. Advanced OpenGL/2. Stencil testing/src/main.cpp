#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"

#define TEXTURES_DIR "../../../../textures/"
#define MODELS_DIR	 "../../../../models/"
#define SHADERS_DIR "../src/shaders/"

using namespace std;
typedef unsigned int uint;

// Global Constants
// ...
// Global Variables
GLFWwindow* window;
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
Camera myCamera(glm::vec3(0.0f, 0.0f, 3.0f));
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float lastX = 400;
float lastY = 300;
bool firstmouse = true;
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

uint8_t init(void);
void framebuffersize_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void proccessInput(GLFWwindow* window);
unsigned int load_texture(const char* path, bool flip = true);

int main(int argc, char** argv)
{
	if (init() != 0)
		return -1;		
	
	Shader shader(SHADERS_DIR "depthTesting.vert", SHADERS_DIR "depthTesting.frag");
	Shader outlineShader(SHADERS_DIR "depthTesting.vert", SHADERS_DIR "outlineShader.frag");

	float cubeVertices[] = {
		// positions          // texture Coords
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	float planeVertices[] = {
		// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,  2.0f, 2.0f
	};

	unsigned int cubeVAO, cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	unsigned int planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	unsigned int cubeTexture = load_texture(TEXTURES_DIR "marble.jpg");
	unsigned int planeTexture = load_texture(TEXTURES_DIR "metal.png");
	
	shader.Use();
	shader.setInt("texture1", 0);

	glEnable(GL_DEPTH_TEST);	// zapne depth test
	glDepthFunc(GL_LESS);		// nastav funkci na blizsi fragmenty
	glEnable(GL_STENCIL_TEST);	// zapne stencil test
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);	// stencil test zobrazi fragment, kdyz hodnota v buffernu neni rovna 1
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);	// pokud stencil nebo depth test selze, tak zanech hodnotu v bufferu, jestli neselze, tak ji nahrad hodnotou 1 (specifikovao v glStencilFunc)
	/* ------------------ MAIN loop ------------------ */
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;	// cas jak dlouho trval posledni frame
		lastFrame = currentFrame;				// cas kdy zacal tento frame
		// input ...
		proccessInput(window);

		// rendering commands here ...
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);		
				
		shader.Use();

		glm::mat4 model(1.0f);
		glm::mat4 view = myCamera.getViewMatrix();
		glm::mat4 projection(1.0f);
		projection = glm::perspective(glm::radians(myCamera.FOV), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);				
				
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		// plane
		glStencilMask(0x00);		// vypne zapisovani do stencil bufferu
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, planeTexture);
		shader.setMat4("model", glm::mat4(1.0f));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// cubes
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		
		glStencilFunc(GL_ALWAYS, 1, 0xff);		// stencil test je pokazde uspesny
		glStencilMask(0xff);					// zapne zapisovani do bufferu (maska je ANDnuta k hodnote, ktera by byla pridana do bufferu)

		// first container
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// second container
		model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glStencilMask(0x00);		// vypne zapis do bufferu ( kazda hodnota, ktera by byla pridana do bufferu skonci jako nula --> x AND 0x00 = 0)
		glStencilFunc(GL_NOTEQUAL, 1, 0xff);	// pokud se hodnota v bufferu nerovna 1, tak test uspeje --> fragment se vykresli
		glDisable(GL_DEPTH_TEST);	// vypne depth test; nechceme aby podlaha prekryvala outline objektu

		outlineShader.Use();
		outlineShader.setMat4("view", view);
		outlineShader.setMat4("projection", projection);				
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, -1.0f));
		model = glm::scale(model, glm::vec3(1.025f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);		
		model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.025f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		/* nastavy stencil testing na defaultni hodnoty */
		glStencilMask(0xff);		// zapne zapis
		glStencilFunc(GL_ALWAYS, 1, 0xff);	// test vydy projde --> vsechny fragmenty se vykresli
		glEnable(GL_DEPTH_TEST);	// zpatky zapne depth test

		// check and calls events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

uint8_t init(void) {
	// init of glfw window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "2. Stencil testing", NULL, NULL);
	if (window == NULL) {
		cerr << "Failed to create glfw window" << endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// init of GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cerr << "Failed to initialize GLAD" << endl;
	}

	glfwSetFramebufferSizeCallback(window, framebuffersize_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	return 0;
}
void framebuffersize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	WINDOW_HEIGHT = height;
	WINDOW_WIDTH = width;
}
void proccessInput(GLFWwindow* window)
{	
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	/* -------------------- Movement -------------------- */
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		myCamera.ProccessKeyboard(CAM_FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		myCamera.ProccessKeyboard(CAM_BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		myCamera.ProccessKeyboard(CAM_RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		myCamera.ProccessKeyboard(CAM_LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		myCamera.ProccessKeyboard(CAM_DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		myCamera.ProccessKeyboard(CAM_UP, deltaTime);

}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstmouse) {
		lastX = xpos;
		lastY = ypos;
		firstmouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	myCamera.ProccessMouse(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	myCamera.ProccessScroll(yoffset);
}
unsigned int load_texture(const char* path, bool flip)
{
	unsigned int texture = 0;
	int width, height, nrChannels, format;
	stbi_set_flip_vertically_on_load(flip);
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);		// load image to the array of bytes (char = 1 byte)	
	
	switch (nrChannels)
	{
	case 4: format = GL_RGBA; break;
	case 3: format = GL_RGB; break;
	case 2: format = GL_RED; break;
	default: format = GL_RGB; break;
	}

	if (data) // data != NULL
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);	// generates texture
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		cerr << "Could not load texture '" << path << "'\n";
	}
	stbi_image_free(data);

	return texture;
}
