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
#include <random>
#include "Shader.h"
#include "Camera.h"
#include "glml/Model.h"
#include "DebugColors.h"
#include "glfbo.h"
#include "config.h"	// in cmake build directory (need to run configure.sh first)
#if _WIN32
#include <Windows.h>
#endif

#define WINDOW_NAME  	"9. Deferred Shading"
#define TEXTURES_DIR	REPO_ROOT "/textures/"
#define MODELS_DIR		REPO_ROOT "/models/"
#define SHADERS_DIR		SOURCE_DIR "/shaders/"
#define _IsUnused		__attribute__((__unused__))		// linux only

using namespace std;
using namespace glfbo;
typedef unsigned int uint;

struct Light {
	glm::vec3 Position, Color, Ambient, Diffuse, Specular;	
};

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
float shader_gamma = 2.2f;
float exposure = 1.0f;

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
Shader* gBufferShader = NULL;
Shader* deferredQuadShader = NULL;

glml::Model* object = NULL;
std::vector<glm::vec3> objectPositions;
std::vector<Light> lights;
const unsigned int NR_LIGHTS = 32;

bool cullFaces = false;
GLint polygonMode = GL_FILL;
glfbo::Framebuffer *printscreen_FBO = NULL;

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
	gBufferShader = new Shader(SHADERS_DIR "shader.vert", SHADERS_DIR "gBufferShader.frag");
	deferredQuadShader = new Shader(SHADERS_DIR "quadShader.vert", SHADERS_DIR "deferredQuadShader.frag");

	if (!shader->Program || !quadShader->Program || !basicShader->Program || !gBufferShader->Program || !deferredQuadShader->Program)
	{
		std::cerr << DC_ERROR " Could not create shader programs!" << std::endl;
		std::cerr << DC_INFO " Exiting...\n";
		return 1;
	}

	//#pragma region "Object positions and light definitions"
	object = new glml::Model(MODELS_DIR "backpack/backpack.obj", true, true);	
    objectPositions.push_back(glm::vec3(-3.0,  -0.5, -3.0));
    objectPositions.push_back(glm::vec3( 0.0,  -0.5, -3.0));
    objectPositions.push_back(glm::vec3( 3.0,  -0.5, -3.0));
    objectPositions.push_back(glm::vec3(-3.0,  -0.5,  0.0));
    objectPositions.push_back(glm::vec3( 0.0,  -0.5,  0.0));
    objectPositions.push_back(glm::vec3( 3.0,  -0.5,  0.0));
    objectPositions.push_back(glm::vec3(-3.0,  -0.5,  3.0));
    objectPositions.push_back(glm::vec3( 0.0,  -0.5,  3.0));
    objectPositions.push_back(glm::vec3( 3.0,  -0.5,  3.0));	

	for (uint i = 0; i < NR_LIGHTS; i++)
	{
		Light l;
		l.Position.x = ((rand() % 100) / 100.0f) * 6.0f - 3.0f;
		l.Position.y = ((rand() % 100) / 100.0f) * 6.0f - 4.0f;
		l.Position.z = ((rand() % 100) / 100.0f) * 6.0f - 3.0f;
		l.Color.r = ((rand() % 100) / 200.0f) + 0.5f;
		l.Color.g = ((rand() % 100) / 200.0f) + 0.5f;
		l.Color.b = ((rand() % 100) / 200.0f) + 0.5f;
		l.Ambient = glm::vec3(0.1f);
		l.Diffuse = glm::vec3(0.8f);
		l.Specular = glm::vec3(1.0f);
		lights.push_back(l);
	}
	//#pragma endregion
	
	myCamera.MovementSpeed = 2.5f;
	
	ColorBuffer fbOffScreen_col0(WINDOW_WIDTH, WINDOW_HEIGHT, BufferType::Texture, GL_RGB16F, 0, GL_CLAMP_TO_EDGE);	
	DepthStencilBuffer fbOffScreen_ds(WINDOW_WIDTH, WINDOW_HEIGHT, BufferType::Renderbuffer, GL_DEPTH24_STENCIL8, 0, GL_CLAMP_TO_EDGE);
	Framebuffer fbOffScreen(0, &fbOffScreen_col0, &fbOffScreen_ds);

	#pragma region gBuffer init
	ColorBuffer gBuffer_pos(WINDOW_WIDTH, WINDOW_HEIGHT, BufferType::Texture, GL_RGBA16F, 0);
	ColorBuffer gBuffer_norm(WINDOW_WIDTH, WINDOW_HEIGHT, BufferType::Texture, GL_RGBA16F, 0);
	ColorBuffer gBuffer_Albedo_spec(WINDOW_WIDTH, WINDOW_HEIGHT, BufferType::Texture, GL_RGBA, 0);
	DepthBuffer gBuffer_Depth(WINDOW_WIDTH, WINDOW_HEIGHT, BufferType::Renderbuffer);
	Framebuffer gBuffer({{0, &gBuffer_pos}, {1, &gBuffer_norm}, {2, &gBuffer_Albedo_spec}}, &gBuffer_Depth, NULL);
	gBuffer.Set_DrawBuffers({0, 1, 2});

	int result;
	if ((result = gBuffer.CheckStatus()) != 0) {
		std::cerr << DC_ERROR "gBuffer isn't complete. Error code: " << result << std::endl;
		return 1;
	}
	#pragma endregion

	printscreen_FBO = &gBuffer;

    if (fbOffScreen.CheckStatus() != 0) {
        std::cout << DC_ERROR << " Could not create fbOffScreen glfbo::MSFramebuffer.\n";
    }

	glEnable(GL_DEPTH_TEST);
	cullFaces = false;	
	//glEnable(GL_MULTISAMPLE);
	/* ------------------ MAIN loop ------------------ */
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;	// cas jak dlouho trval posledni frame v sekundach
		lastFrame = currentFrame;				// cas kdy zacal tento frame    

		// input ...
		proccessInput(window);

		glEnable(GL_DEPTH_TEST);
		if (cullFaces)
			glEnable(GL_CULL_FACE);

		#pragma region "1. Render pass: render to gBuffer framebuffer"
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glPolygonMode(GL_FRONT_AND_BACK, polygonMode);		
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.ID);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);        
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		gBufferShader->Use();
		draw_scene(gBufferShader, false);
		#pragma endregion

		#pragma region "2. Render pass: render quad on screen using gBuffer textures"
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		deferredQuadShader->Use();
		deferredQuadShader->setInt("gBuffer.texture_position", 0);
		deferredQuadShader->setInt("gBuffer.texture_normal", 1);
		deferredQuadShader->setInt("gBuffer.texture_albedo_spec", 2);
		for (int i = 0; i < 3; i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, gBuffer.Get_ColorBuffer(i));
		}

		/* Lighting */
		deferredQuadShader->setVec3("viewPos", myCamera.Position);
		deferredQuadShader->setInt("lights_num", lights.size());
		for (int i = 0; i < lights.size(); i++)
		{
			deferredQuadShader->setVec3("lights[" + std::to_string(i) + "].position", lights[i].Position);		
			deferredQuadShader->setVec3("lights[" + std::to_string(i) + "].ambient", lights[i].Ambient);
			deferredQuadShader->setVec3("lights[" + std::to_string(i) + "].diffuse", lights[i].Diffuse * lights[i].Color);
			deferredQuadShader->setVec3("lights[" + std::to_string(i) + "].specular", lights[i].Specular * lights[i].Color);	
		}

		draw_quad();
		#pragma endregion

		glDisable(GL_FRAMEBUFFER_SRGB);

		// check and calls events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwTerminate();	
	delete(shader);
	delete(quadShader);	
	delete(basicShader);	
	delete(object);	

	return 0;
}
unsigned int wood_texture = 0, container_texture = 0;
void draw_scene(const Shader* shader, bool lightCube)
{
	if (wood_texture == 0)									   // flipV sRGB
		wood_texture = load_texture(TEXTURES_DIR "wood/wood.png", true, true);	
	if (container_texture == 0)
		container_texture = load_texture(TEXTURES_DIR "container2.png", true, true);

	glm::mat4 model(1.0f);
	glm::mat4 view = myCamera.getViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(myCamera.FOV), 
		WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 300.0f);
	

	shader->Use();
	shader->setFloat("gamma", shader_gamma);
	shader->setMat4("projection", projection);
	shader->setMat4("view", view);	

	//CUBES HERE	

	if (cullFaces)
		glEnable(GL_CULL_FACE);

	for (int i = 0; i < objectPositions.size(); i++)
	{
		model = glm::mat4(1.0f);
		model = glm::translate(model, objectPositions[i]);
		model = glm::scale(model, glm::vec3(0.5f));
		shader->setMat4("model", model);
		shader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));	
		object->Draw(*shader);
	}

	
	if (lightCube)
	{
		for (int i = 0; i < lights.size(); i++)
		{
			basicShader->Use();
			model = glm::translate(glm::mat4(1.0f), lights[i].Position);
			model = glm::scale(model, glm::vec3(0.1f));
			basicShader->setMat4("PVM", projection * view * model);
			basicShader->setVec3("Color", lights[i].Color);
			draw_cube();
		}
	}
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
	
	glfwSwapInterval(1);

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
			polygonMode = GL_FILL;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)			// F2
	{
		if (polygonMode != GL_LINE)
		{			
			polygonMode = GL_LINE;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)			// F3
	{
		if (printscreen_FBO)
		{			
			string s = (printscreen_FBO)->Export("", ImageType::PNG, 0);
			if (s.length() != 0)
				std::cout << DC_INFO << " Screenshot saved to '" DC_MAGNETA BINARY_DIR "/" << s << DC_DEFAULT "'\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}	
	if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS)			// F4		
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	float exposure_gain_speed = 1.0f; // gain 1.0 per second
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		exposure += exposure_gain_speed * deltaTime;	
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		if (exposure > 0.0f) {
			exposure -= exposure_gain_speed * deltaTime;	
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
		float quadVertices[] = {
			// positions            // normal         // texcoords  
			-1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,	0.0f, 0.0f,
			1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
			1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,   0.0f, 0.0f, 1.0f,	1.0f, 1.0f
		};
		// configure plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0); // Vertex positions
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1); // Vertex normals
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2); // Vertex texture coordinates
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));		
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
