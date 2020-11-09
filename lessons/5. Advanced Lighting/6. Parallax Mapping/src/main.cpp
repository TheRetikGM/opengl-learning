#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <stb_image_write.h>
#include <ctime>
#include <thread>
#include <chrono>
#include "Shader.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "Model.h"
#include "DebugColors.h"
#include "config.h"	// in cmake build directory (need to run configure.sh first)
#if _WIN32
	#include <Windows.h>
#endif

#define WINDOW_NAME  	"6. Parallax Mapping"
#define TEXTURES_DIR	REPO_ROOT "/textures/"
#define MODELS_DIR		REPO_ROOT "/models/"
#define SHADERS_DIR		SOURCE_DIR "/shaders/"
#define _IsUnused		__attribute__((__unused__))		// linux only

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

/* pointer to Framebuffer object that is used for screenshot */
/* if NULL default framebuffer is used */
Framebuffer *screenshot_FBO_ptr = NULL;

uint8_t init(void);
void framebuffersize_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void proccessInput(GLFWwindow* window);
unsigned int load_texture(const char* path, bool flip = true, bool sRGB = true, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT);
unsigned int load_cubemap(vector<string> faces);
void draw_cube(void);
void draw_floor(void);
void draw_quad(void);
void draw_scene(const Shader* shader, bool lightCube);

Shader* shader = NULL;
Shader* quadShader = NULL;
Shader* basicShader = NULL;

glm::vec3 lightPos(0.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
bool cullFaces = false;
bool normalMapping = false;
bool parallaxMapping = false;
GLint polygonMode = GL_FILL;

int main(int argc, char** argv)
{
#if _WIN32	
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

	if (init() != 0)
		return -1;		

	shader = new Shader(SHADERS_DIR "shader.vert", SHADERS_DIR "shader.frag");
	quadShader = new Shader(SHADERS_DIR "quadShader.vert", SHADERS_DIR "quadShader.frag");
	basicShader = new Shader(SHADERS_DIR "basicShader.vert", SHADERS_DIR "basicShader.frag");	

	if (!shader->Program || !quadShader->Program || !basicShader->Program)
	{
		std::cerr << DC_ERROR " Could not create shader programs!" << std::endl;
		std::cerr << DC_INFO " Exiting...\n";
		return -2;
	}	
	
	MSFramebuffer fbOffScreen(WINDOW_WIDTH, WINDOW_HEIGHT, 4);
	if (fbOffScreen.ID == 0)
		return -1;			
	screenshot_FBO_ptr = &fbOffScreen;

	glEnable(GL_DEPTH_TEST);
	cullFaces = false;	
	//glEnable(GL_MULTISAMPLE);
	/* ------------------ MAIN loop ------------------ */
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;	// cas jak dlouho trval posledni frame
		lastFrame = currentFrame;				// cas kdy zacal tento frame
		// input ...
		proccessInput(window);
		// rendering commands here ...
		glEnable(GL_DEPTH_TEST);
		if (cullFaces)
			glEnable(GL_CULL_FACE);

		// 1. Render pass: render to fbOffscreen framebuffer
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
		fbOffScreen.Use();
		fbOffScreen.clearBuffers(0.006f, 0.006f, 0.006f, 1.0f);		

		shader->Use();		
		draw_scene(shader, false);				
		
		// 2. Render pass: render quad on screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		quadShader->Use();		
		quadShader->setInt("texture0", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fbOffScreen.getColorBuffer());
		draw_quad();

		//glDisable(GL_FRAMEBUFFER_SRGB);

		// check and calls events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwTerminate();

	delete(shader);
	delete(quadShader);	
	delete(basicShader);

	return 0;
}

uint8_t init(void) {
	// init of glfw window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, NULL, NULL);
	if (window == NULL) {
		cerr << DC_ERROR " Failed to create glfw window" << endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// init of GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cerr << DC_ERROR " Failed to initialize GLAD" << endl;
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
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)		// Escape
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)			// F1
	{
		if (polygonMode != GL_FILL)
		{
			std::cout << DC_INFO << " Polygon mode: " << DC_YELLOW << "fill" << DC_DEFAULT << "\n";
			polygonMode = GL_FILL;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)			// F2
	{
		if (polygonMode != GL_LINE)
		{
			std::cout << DC_INFO << " Polygon mode: " << DC_YELLOW << "line" << DC_DEFAULT << "\n";
			polygonMode = GL_LINE;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)			// F3
	{
		std::string path;
		if (screenshot_FBO_ptr == NULL)
			path = Framebuffer::screenshot_defaultFBO(WINDOW_WIDTH, WINDOW_HEIGHT);			
		else 
			path = screenshot_FBO_ptr->screenshot();	

		std::this_thread::sleep_for(std::chrono::seconds(1));		
		std::cout << DC_INFO << " Screenshot saved to '" DC_MAGNETA << path << DC_DEFAULT "'\n";
	}	
	//if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS);			// F4		
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)			// 1
	{
		if (!normalMapping) {
			std::cout << DC_INFO << " Normal mapping: " << DC_GREEN << "on" << DC_DEFAULT << "\n";
			normalMapping = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)			// 2
	{
		if (normalMapping) {
			std::cout << DC_INFO << " Normal mapping: " << DC_RED << "off" << DC_DEFAULT << "\n";
			normalMapping = false;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		if (!parallaxMapping) {
			std::cout << DC_INFO << " Parallax mapping: " << DC_GREEN << "on" << DC_DEFAULT << "\n";
			parallaxMapping = true;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
	{
		if (parallaxMapping) {
			std::cout << DC_INFO << " Parallax mapping: " << DC_RED << "off" << DC_DEFAULT << "\n";
			parallaxMapping = false;
		}
	}
	/* -------------------- Movement -------------------- */
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)			// w
		myCamera.ProccessKeyboard(CAM_FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)			// S
		myCamera.ProccessKeyboard(CAM_BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)			// D
		myCamera.ProccessKeyboard(CAM_RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)			// A
		myCamera.ProccessKeyboard(CAM_LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)	// Left-shift
		myCamera.ProccessKeyboard(CAM_DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)		// Space
		myCamera.ProccessKeyboard(CAM_UP, deltaTime);

}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstmouse) {
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstmouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos;
	lastX = (float)xpos;
	lastY = (float)ypos;

	myCamera.ProccessMouse(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	myCamera.ProccessScroll((float)yoffset);
}
unsigned int load_texture(const char* path, bool flip, bool sRGB, GLint wrapS, GLint wrapT)
{
	unsigned int texture = 0;
	int width, height, nrChannels, format, intFormat;
	stbi_set_flip_vertically_on_load(flip);
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);		// load image to the array of bytes (char = 1 byte)	
	
	switch (nrChannels)
	{
	case 4: format = GL_RGBA; 
		if (sRGB) intFormat = GL_SRGB_ALPHA;
		else intFormat = GL_RGBA;
	break;
	case 3: format = GL_RGB; 
		if (sRGB) intFormat = GL_SRGB;
		else intFormat = GL_RGB;
	break;
	case 2: format = intFormat = GL_RED; break;
	default: format = intFormat = GL_RGB; break;
	}

	if (data) // data != NULL
	{
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);	// generates texture
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		cerr << DC_WARNING " Could not load texture '" << path << "'\n";
	}
	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}
unsigned int load_cubemap(vector<string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels, format;
	for (size_t i = 0; i < faces.size(); i++)
	{
			unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
			switch (nrChannels)
			{
			case 4: format = GL_RGBA; break;
			case 3: format = GL_RGB; break;
			case 2: format = GL_RED; break;
			default: format = GL_RGB; break;
			}
			if (data)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<int>(i), 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			}
			else {
				std::cerr << DC_INFO " Failed to load cubemap texture at " << faces[i] << std::endl;
			}
			stbi_image_free(data);
	};
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

unsigned int cubeVAO = 0, cubeVBO = 0;
void draw_cube(void)
{
	if (cubeVBO == 0 || cubeVAO == 0)
	{
		float cubeVertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};

		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindVertexArray(cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(6 * sizeof(float)));
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
		for (int i = 0; i < 3; i++)
			glEnableVertexAttribArray(i);
		glBindVertexArray(0);
	}
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

unsigned int floorVAO = 0, floorVBO = 0;
void draw_floor(void)
{
	if (floorVAO == 0 || floorVBO == 0)
	{
		float floorVertices[] = {
			// positions            // normals         // texcoords
		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
		};

		glGenVertexArrays(1, &floorVAO);
		glGenBuffers(1, &floorVBO);
		glBindVertexArray(floorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0); // aPos
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)0);
		glEnableVertexAttribArray(1); // aTexCoord		
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2); // aNormal		
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
		glBindVertexArray(0);
	}
	glBindVertexArray(floorVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

unsigned int quadVAO = 0, quadVBO = 0;
void draw_quad(void)
{
	if (quadVAO == 0 || quadVBO == 0)
	{
		//Tangent space calculations 	
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
		glm::vec3 pos4(1.0f, 1.0f, 0.0f);
		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);
		// normal vector
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// triangle 1
		// ----------
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		// triangle 2
		// ----------
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		float quadVertices[] = {
			// positions            // normal         // texcoords  // tangent                          // bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};
		// configure plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0); // Vertex positions
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1); // Vertex normals
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2); // Vertex texture coordinates
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3); // Vertex tangent
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(4); // Vertex bitangent
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

unsigned int brick_texture = 0, brick_texture_normal = 0, brick_texture_height = 0;
void draw_scene(const Shader* shader, bool lightCube)
{
	if (brick_texture == 0)												  // flipV sRGB
		brick_texture = load_texture(TEXTURES_DIR "brickwall2/brickwall2.jpg", true, true);	
	if (brick_texture_normal == 0)
		brick_texture_normal = load_texture(TEXTURES_DIR "brickwall2/brickwall2_normal.jpg", true, false);
	if (brick_texture_height == 0)
		brick_texture_height = load_texture(TEXTURES_DIR "brickwall2/brickwall2_height.jpg", true, false);
	if (!brick_texture || !brick_texture_normal || !brick_texture_height)
	{
		std::cout << DC_ERROR " Could not load needed textures. Exiting..." DC_DEFAULT << std::endl;		
		exit(EXIT_FAILURE);
	}
	glm::mat4 model(1.0f);
	glm::mat4 view = myCamera.getViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(myCamera.FOV), 
		WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 300.0f);

	shader->Use();	
	shader->setMat4("projection", projection);
	shader->setMat4("view", view);	
	
	// Lighting 
	float tim = (float)glfwGetTime();
	lightPos = glm::vec3(0.0f, 0.0f, 1.5f);
	shader->setVec3("light.position", lightPos);
	shader->setVec3("viewPos", myCamera.Position);
	shader->setVec3("light.ambient", glm::vec3(0.005f));
	shader->setVec3("light.diffuse", 0.9f * lightColor);
	shader->setVec3("light.specular", 1.0f * lightColor);

	shader->setInt("material.texture_diffuse0", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, brick_texture);
	shader->setInt("material.texture_normal0", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, brick_texture_normal);
	shader->setInt("material.texture_height0", 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, brick_texture_height);

	if (cullFaces)
		glEnable(GL_CULL_FACE);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
	model = glm::scale(model, glm::vec3(1.0f));
	//model = glm::rotate(model, tim * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));	
	shader->setMat4("model", model);
	shader->setMat3("normalMatrix", glm::transpose(glm::inverse(model)));
	shader->setBool("normalMapping", normalMapping);
	shader->setBool("parallaxMapping", parallaxMapping);
	shader->setFloat("height_scale", 0.15f);
	draw_quad();

	if (lightCube)
	{
		basicShader->Use();
		model = glm::translate(glm::mat4(1.0f), lightPos);
		model = glm::scale(model, glm::vec3(0.05f));
		basicShader->setMat4("PVM", projection * view * model);
		basicShader->setVec3("Color", lightColor);
		draw_cube();
	}
}