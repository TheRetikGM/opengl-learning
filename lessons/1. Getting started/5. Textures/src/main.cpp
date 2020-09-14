#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stb_image.h>
#include "Shader.h"

#define TEXTURES_DIR "../../../../textures/"

using namespace std;
typedef unsigned int uint;

// Global Constants
const int WINDOW_WIDTH = 800; 
const int WINDOW_HEIGHT = 600;
// Global Variables
GLFWwindow* window;
float ratio = 0.2;

uint8_t init(void);
void framebuffersize_callback(GLFWwindow* window, int width, int height);
void proccessInput(GLFWwindow* window);
unsigned int load_texture(const char* path, bool flip = true);

int main(int* argc, char** argv)
{
	if (init() != 0)
		return -1;

	Shader myShader("../src/shaders/vertexShader.vert", "../src/shaders/fragmentShader.frag");

	float vertices[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	uint VAO;	// Vertex array buffer
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	uint VBO;	// Vertex Buffer Object
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);	// copy data to Graphics card array buffer (buffer for vertices)	

	uint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	
	uint texture1 = load_texture(TEXTURES_DIR "container.jpg", false);
	uint texture2 = load_texture(TEXTURES_DIR "awesomeface.png");

	myShader.Use();
	myShader.setInt("texture1", 0);		// set Texture units IDs
	myShader.setInt("texture2", 1);

	/* ------------------ MAIN loop ------------------ */
	while (!glfwWindowShouldClose(window)) {
		// input ...
		proccessInput(window);

		// rendering commands here ...
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		myShader.Use();

		myShader.setFloat("ratio", ratio);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// check and calls events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);	
	glfwTerminate();

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
void proccessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (ratio < 1.0)
			ratio += 0.001f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (ratio > 0.0)
			ratio -= 0.001f;
	}

}
unsigned int load_texture(const char* path, bool flip)
{
	unsigned int texture = 0;
	int width, height, nrChannels, format;
	stbi_set_flip_vertically_on_load(flip);
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);		// load image to the array of bites (char = 1 byte)	
	
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
