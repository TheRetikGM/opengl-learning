#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <map>
#include <algorithm>
#include "Shader.h"
#include "Camera.h"
#include "Framebuffer.h"

#define TEXTURES_DIR "../../../../textures/"
#define MODELS_DIR	 "../../../../models/"
#define SHADERS_DIR	"../src/shaders/"

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
unsigned int load_texture(const char* path, bool flip = true, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT);

int main(int argc, char** argv)
{
	if (init() != 0)
		return -1;		
	
	Shader shader(SHADERS_DIR "shader.vert", SHADERS_DIR "shader.frag");
	Shader quadShader(SHADERS_DIR "quadShader.vert", SHADERS_DIR "quadShader.frag");

	//// Framebuffer operations
	//unsigned int FBO;	// Frame buffer object
	//glGenFramebuffers(1, &FBO);
	//glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	//unsigned int texColorBuffer;
	//glGenTextures(1, &texColorBuffer);
	//glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glBindTexture(GL_TEXTURE_2D, 0);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);		// attach texture buffer to framebuffer

	//// render buffer for depth testing and stencil testing
	//unsigned int RBO;	// Render Buffer Object
	//glGenRenderbuffers(1, &RBO);
	//glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	//// Attach render buffer to framebuffer
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	//	std::cerr << "[FRAMEBUFFER ERROR] Framebuffer is incomplete!\nError code ";

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);	// bind default framebuffer

	Framebuffer framebuffer(WINDOW_WIDTH, WINDOW_HEIGHT);	

	float cubeVertices[] = {
		// back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right    
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right              
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left                
		// front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right        
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left        
		// left face
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left       
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		// right face
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right      
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right          
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		// bottom face          
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left        
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		// top face
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right                 
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, // bottom-left  
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f  // top-left              
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
	float quadVertices[] = {
		-1.0f,  1.0f,	0.0f, 1.0f,
		-1.0f, -1.0f,	0.0f, 0.0f,
		 1.0f, -1.0f,	1.0f, 0.0f,

		-1.0f,  1.0f,	0.0f, 1.0f,
	     1.0f, -1.0f,	1.0f, 0.0f,
	     1.0f,  1.0f,	1.0f, 1.0f
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

	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_TRUE, 4 * sizeof(float), (const void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	unsigned int cubeTexture = load_texture(TEXTURES_DIR "container.jpg");
	unsigned int planeTexture = load_texture(TEXTURES_DIR "metal.png");	
	
	shader.Use();
	shader.setInt("texture1", 0);
	quadShader.Use();
	quadShader.setInt("quadTexture", 0);

	//glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	/* ------------------ MAIN loop ------------------ */
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;	// cas jak dlouho trval posledni frame
		lastFrame = currentFrame;				// cas kdy zacal tento frame
		// input ...
		proccessInput(window);
		// rendering commands here ...

		framebuffer.Use();
		framebuffer.clearBuffers();
		glEnable(GL_DEPTH_TEST);
				
		shader.Use();
		glm::mat4 model(1.0f);
		glm::mat4 view = myCamera.getViewMatrix();
		glm::mat4 projection(1.0f);
		projection = glm::perspective(glm::radians(myCamera.FOV), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
		
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		// cubes		
		glBindVertexArray(cubeVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cubeTexture);
		model = glm::translate(model, glm::vec3(-1.0f, 0.05f, -1.0f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.05f, 0.0f));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glDisable(GL_CULL_FACE);
		// plane
		glBindVertexArray(planeVAO);
		glBindTexture(GL_TEXTURE_2D, planeTexture);
		shader.setMat4("model", glm::mat4(1.0f));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);	

		/* DRAW TO ACTUAL SCREEN */
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);		// disable depth test, so that the quad on the screen wont get replaced
				
		quadShader.Use();
		quadShader.setMat4("PVM", glm::mat4(1.0f));
		glBindVertexArray(quadVAO);		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.ColorBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);		

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
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "5. Framebuffers", NULL, NULL);
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
unsigned int load_texture(const char* path, bool flip, GLint wrapS, GLint wrapT)
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
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);	// generates texture
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		cerr << "Could not load texture '" << path << "'\n";
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}
