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
#include "Shader.h"
#include "Camera.h"
#include "Framebuffer.h"
#include "DebugColors.h"
//#include <unistd.h>
#include "config.h"	// in cmake build directory (need to run configure.sh first)

#define WINDOW_NAME  	"4. Point Shadow"
#define TEXTURES_DIR	REPO_ROOT "/textures/"
#define MODELS_DIR		REPO_ROOT "/models/"
#define SHADERS_DIR		SOURCE_DIR "/shaders/"
#define _IsUnused		__attribute__((__unused__))		// linux only

using namespace std;
typedef unsigned int uint;

//STBIWDEF int stbi_write_png(char const *filename, int x, int y, int comp, const void *data, int stride_bytes);

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
Shader* depthmapShader = NULL;

glm::vec3 lightPos(0.0f);

int main(int argc, char** argv)
{
	if (init() != 0)
		return -1;	

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);

	shader = new Shader(SHADERS_DIR "shader.vert", SHADERS_DIR "shader.frag");
	quadShader = new Shader(SHADERS_DIR "quadShader.vert", SHADERS_DIR "quadShader.frag");
	basicShader = new Shader(SHADERS_DIR "basicShader.vert", SHADERS_DIR "basicShader.frag");
	depthmapShader = new Shader(SHADERS_DIR "depthmapShader.vert", SHADERS_DIR "depthmapShader.frag",
		SHADERS_DIR "depthmapShader.geom");

	if (!shader->Program || !quadShader->Program || !basicShader->Program || !depthmapShader->Program)
	{
		std::cerr << ERROR " Could not create shader programs!" << std::endl;
		std::cerr << INFO " Exiting...\n";
		return -2;
	}	
	
	MSFramebuffer fbOffScreen(WINDOW_WIDTH, WINDOW_HEIGHT, 4);
	if (fbOffScreen.ID == 0)
		return -1;			
	screenshot_FBO_ptr = &fbOffScreen;


	unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	unsigned int depthmapFBO;
	glGenFramebuffers(1, &depthmapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthmapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	DepthmapFramebuffer fbo_depthmap(2048, 2048);
	if (fbo_depthmap.ID == 0)
		return -1;

	float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
	float near_plane = 1.0f;
	float far_plane = 25.0f;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near_plane, far_plane);
	glm::mat4 shadowTransforms[6] = {
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
		shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_MULTISAMPLE);
	/* ------------------ MAIN loop ------------------ */
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;	// cas jak dlouho trval posledni frame
		lastFrame = currentFrame;				// cas kdy zacal tento frame
		// input ...
		proccessInput(window);
		// rendering commands here ...
		glEnable(GL_DEPTH_TEST);
		// 1. Render pass: rendering the scene into depth map (shadow map)

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		depthmapShader->Use();
		for (int i = 0; i < 6; i++)		
			depthmapShader->setMat4("shadowTransforms[" + to_string(i) + "]", shadowTransforms[i]);
		depthmapShader->setFloat("far_plane", far_plane);
		depthmapShader->setVec3("lightPos", lightPos);
		glBindFramebuffer(GL_FRAMEBUFFER, depthmapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			draw_scene(depthmapShader, false);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. Render pass: render to fbOffscreen framebuffer
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		fbOffScreen.Use();
		fbOffScreen.clearBuffers(0.006f, 0.006f, 0.006f, 1.0f);		

		shader->Use();
		shader->setFloat("far_plane", far_plane);
		shader->setInt("depthCubemap", 1);		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		glCullFace(GL_FRONT);
		draw_scene(shader, true);
		glCullFace(GL_BACK);
		
		// 3. Render pass: render quad on screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);								
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		//glEnable(GL_FRAMEBUFFER_SRGB);

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
	delete(depthmapShader);
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
		cerr << ERROR " Failed to create glfw window" << endl;
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// init of GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		cerr << ERROR " Failed to initialize GLAD" << endl;
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
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)			// F2
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)			// F3
	{
		if (screenshot_FBO_ptr == NULL)
			Framebuffer::screenshot_defaultFBO(WINDOW_WIDTH, WINDOW_HEIGHT);
		else
			screenshot_FBO_ptr->screenshot();
	}	
	if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS)			// F4
		Framebuffer::screenshot_defaultFBO(WINDOW_WIDTH, WINDOW_HEIGHT);
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS);			// 1
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS);			// 2
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
		cerr << WARNING " Could not load texture '" << path << "'\n";
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
				std::cerr << INFO " Failed to load cubemap texture at " << faces[i] << std::endl;
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
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(6 * sizeof(float)));
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
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(6 * sizeof(float)));
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
		float quadVertices[] = {
			// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
			 1.0f,  1.0f,  1.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 0.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,

			 1.0f,  1.0f,  1.0f, 1.0f,
			-1.0f, -1.0f,  0.0f, 0.0f,
			-1.0f,  1.0f,  0.0f, 1.0f
		};
		
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(2 * sizeof(float)));
		glBindVertexArray(0);
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

unsigned int wood_texture = 0, wood_specular_texture = 0;
void draw_scene(const Shader* shader, bool lightCube)
{
	if (wood_texture == 0)									   // flip  sRGB
		wood_texture = load_texture(TEXTURES_DIR "wood/wood.png", true, true);	

	glm::mat4 model(1.0f);
	glm::mat4 view = myCamera.getViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(myCamera.FOV), 
		WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 300.0f);

	shader->Use();	
	shader->setMat4("projection", projection);
	shader->setMat4("view", view);	
	
	// Lighting 	
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	
	shader->setVec3("light.position", lightPos);
	shader->setVec3("viewPos", myCamera.Position);
	shader->setVec3("light.ambient", glm::vec3(0.005f));
	shader->setVec3("light.diffuse", 0.9f * lightColor);
	shader->setVec3("light.specular", 0.0707f * lightColor);

	shader->setInt("texture0", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, wood_texture);

	// floor	
	//shader->setMat4("model", model);
	//shader->setMat3("normalMatrix", glm::transpose(glm::inverse(model)));
	//draw_floor();	
	model = glm::scale(model, glm::vec3(5.0));
	shader->setMat4("model", model);
	shader->setMat3("normalMatrix", glm::transpose(glm::inverse(model)));
	glDisable(GL_CULL_FACE);
	shader->setBool("reverseNormal", true);
	draw_cube();
	shader->setBool("reverseNormal", false);
	glEnable(GL_CULL_FACE);	
	// cubes	
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(4.0f, -3.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader->setMat4("model", model);
	shader->setMat3("normalMatrix", glm::transpose(glm::inverse(model)));
	draw_cube();	
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 3.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.75f));
	shader->setMat4("model", model);
	shader->setMat3("normalMatrix", glm::transpose(glm::inverse(model)));
	draw_cube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-3.0f, -1.0f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader->setMat4("model", model);
	shader->setMat3("normalMatrix", glm::transpose(glm::inverse(model)));
	draw_cube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, 1.0f, 1.5));
	model = glm::scale(model, glm::vec3(0.5f));
	shader->setMat4("model", model);
	shader->setMat3("normalMatrix", glm::transpose(glm::inverse(model)));
	draw_cube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.75f));
	shader->setMat4("model", model);
	shader->setMat3("normalMatrix", glm::transpose(glm::inverse(model)));
	draw_cube();

	if (lightCube)
	{
		basicShader->Use();
		model = glm::translate(glm::mat4(1.0f), lightPos);
		model = glm::scale(model, glm::vec3(0.1f));
		basicShader->setMat4("PVM", projection * view * model);
		basicShader->setVec3("Color", lightColor);
		draw_cube();
	}
}