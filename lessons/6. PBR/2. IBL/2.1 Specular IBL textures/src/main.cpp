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
#define _INSTALL
#include "config.h"	// in cmake build directory (need to run configure.sh first)
#if _WIN32
#include <Windows.h>
#endif

#define WINDOW_NAME  	"6. PBR/Lighting"
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

struct PBRTextureSet {
	unsigned int albedo, ao, metallic, normal, roughness;
	unsigned int irradianceMap, prefilterMap, brdfLUT;

	PBRTextureSet(std::string path, unsigned int irradianceMap, unsigned int pefilterMap, unsigned int brdfLUT);
	~PBRTextureSet() {
		unsigned int texs[] = { albedo, ao, metallic, normal, roughness };
		glDeleteTextures(5, texs);
	}
	void Set(Shader* s);
	void Bind();
};

// Global Constants
// ...
// Global Variables
GLFWwindow* window;
int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
Camera myCamera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0));
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
unsigned int load_hdr_image(const char* path, bool flip = true);
void draw_cube(void);
void draw_floor(void);
void draw_quad(void);
void draw_sphere(void);
float lerp(float a, float b, float f);

Shader* quadShader = nullptr;
Shader* basicShader = nullptr;
Shader* pbrShader = nullptr;
Shader* pbrShader_tex = nullptr;

GLint polygonMode = GL_FILL;
glfbo::Framebuffer *printscreen_FBO = NULL;

int main(int argc, char** argv)
{
#if _WIN32	
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

	if (init() != 0)
		return -1;	
	
	quadShader = new Shader(SHADERS_DIR "quadShader.vert", SHADERS_DIR "quadShader.frag");
	basicShader = new Shader(SHADERS_DIR "basicShader.vert", SHADERS_DIR "basicShader.frag");		
	pbrShader = new Shader(SHADERS_DIR "pbrShader.vert", SHADERS_DIR "pbrShader.frag");	
	Shader hdrToCubemap(SHADERS_DIR "hdrToCubemap.vert", SHADERS_DIR "hdrToCubemap.frag");
	Shader skyboxShader(SHADERS_DIR "skyboxShader.vert", SHADERS_DIR "skyboxShader.frag");
	Shader irradianceShader(SHADERS_DIR "skyboxShader.vert", SHADERS_DIR "irradianceShader.frag");
	Shader prefilterShader(SHADERS_DIR "skyboxShader.vert", SHADERS_DIR "prefilterConvShader.frag");
	Shader brdfShader(SHADERS_DIR "quadShader.vert", SHADERS_DIR "brdfConvolution.frag");

	if (!quadShader->Program || !basicShader->Program || !pbrShader->Program ||
		!hdrToCubemap.Program || !skyboxShader.Program || !irradianceShader.Program || !prefilterShader.Program || !brdfShader.Program)
	{
		std::cerr << DC_ERROR " Could not create shader programs!" << std::endl;
		std::cerr << DC_INFO " Exiting...\n";
		return 1;
	}
		
	/*pbrShader_tex->setInt("albedoMap", 0);
	pbrShader_tex->setInt("normalMap", 1);
	pbrShader_tex->setInt("metallicMap", 2);
	pbrShader_tex->setInt("roughnessMap", 3);
	pbrShader_tex->setInt("aoMap", 4);*/
	glm::vec3 lightPositions[] = {
		glm::vec3(-10.0f,  10.0f, 10.0f),
		glm::vec3(10.0f,  10.0f, 10.0f),
		glm::vec3(-10.0f, -10.0f, 10.0f),
		glm::vec3(10.0f, -10.0f, 10.0f),
	};
	glm::vec3 lightColors[] = {
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f)
	};
	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 2.5;
	
	myCamera.MovementSpeed = 4.3f;
	printscreen_FBO = NULL;

	#pragma region Convert equiretangular map to cubemap
	unsigned int hdrTexture = load_hdr_image(TEXTURES_DIR "10-Shiodome_Stairs_3k.hdr", true);

	ColorBuffer envCubemap(512, 512, BufferType::Cubemap, GL_RGB16F, 0, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap.ID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	DepthBuffer capture_depth(512, 512, BufferType::Renderbuffer, GL_DEPTH_COMPONENT24);
	Framebuffer captureFBO(0, &envCubemap, &capture_depth, NULL);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureView[] = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
	};

	hdrToCubemap.Use();
	hdrToCubemap.setMat4("projection", captureProjection);	
	hdrToCubemap.setInt("equiretangularMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	
	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO.ID);

	for (unsigned int i = 0; i < 6; i++)
	{
		hdrToCubemap.setMat4("view", captureView[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap.ID, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_cube();
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap.ID);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ColorBuffer irradianceMap(32, 32, BufferType::Cubemap, GL_RGB16F, 0, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap.ID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// resize the capture_depth renderbuffer
	glBindRenderbuffer(GL_RENDERBUFFER, capture_depth.ID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	irradianceShader.Use();
	irradianceShader.setInt("environmentMap", 0);
	irradianceShader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap.ID);

	glViewport(0, 0, 32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO.ID);
	for (unsigned int i = 0; i < 6; i++)
	{
		irradianceShader.setMat4("view", captureView[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap.ID, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_cube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	#pragma endregion

	#pragma region Pre-filtered map

	unsigned int prefilterMap;
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	prefilterShader.Use();
	prefilterShader.setInt("environmentMap", 0);
	prefilterShader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap.ID);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO.ID);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, capture_depth.ID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader.setFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader.setMat4("view", captureView[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			draw_cube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// generate brdf lut texture
	unsigned int brdfLutTexture;
	glGenTextures(1, &brdfLutTexture);
	glBindTexture(GL_TEXTURE_2D, brdfLutTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO.ID);
	glBindRenderbuffer(GL_RENDERBUFFER, capture_depth.ID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLutTexture, 0);

	glViewport(0, 0, 512, 512);
	brdfShader.Use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_quad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	#pragma endregion

	std::vector<std::string> pbrTextureSetDirs = {		
		TEXTURES_DIR "pbr/rusted_iron/",		
		TEXTURES_DIR "pbr/gold/",
		TEXTURES_DIR "pbr/grass/",				
		TEXTURES_DIR "pbr/plastic/",
		TEXTURES_DIR "pbr/wall/",
	};
	std::vector<PBRTextureSet*> PBRSets;
	int iter = 0;
	for (auto& i : pbrTextureSetDirs) {
		PBRSets.push_back(new PBRTextureSet(i, irradianceMap.ID, prefilterMap, brdfLutTexture));
		PBRSets[iter]->Set(pbrShader);
		std::cout << DC_INFO " Loaded PBR texture: " << i << "\n";
		iter++;
	}

	glEnable(GL_DEPTH_TEST);	
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	//glEnable(GL_MULTISAMPLE);
	/* ------------------ MAIN loop ------------------ */
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;	// cas jak dlouho trval posledni frame v sekundach
		lastFrame = currentFrame;				// cas kdy zacal tento frame    		

		// input ...
		proccessInput(window);		
		
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(myCamera.FOV), WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 50.0f);
		glm::mat4 view = myCamera.getViewMatrix();
		glm::mat4 model(1.0f);				

		for (int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); i++) {
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
			pbrShader->Use();
			pbrShader->setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
			pbrShader->setVec3("lights[" + std::to_string(i) + "].Position", newPos);

			model = glm::translate(glm::mat4(1.0f), newPos);
			model = glm::scale(model, glm::vec3(0.5f));
			basicShader->Use();
			basicShader->setMat4("PVM", projection * view * model);
			basicShader->setVec3("Color", lightColors[i]);			
			draw_sphere();
		}	
		pbrShader->Use();
		pbrShader->setMat4("projection", projection);
		pbrShader->setMat4("view", view);
		pbrShader->setVec3("camPos", myCamera.Position);			
		
		nrRows = 1;
		nrColumns = PBRSets.size();
		for (int row = 0; row < nrRows; ++row)
		{			
			for (int col = 0; col < nrColumns; ++col)
			{				
				PBRSets[col]->Bind();

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(
					(col - (nrColumns / 2)) * spacing,
					(row - (nrRows / 2)) * spacing,
					0.0f
				));
				pbrShader->setMat4("model", model);
				pbrShader->setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
				draw_sphere();
			}
		}				

		glDepthFunc(GL_LEQUAL);
		skyboxShader.Use();
		skyboxShader.setMat4("projection", projection);
		skyboxShader.setMat4("view", view);
		skyboxShader.setInt("environmentMap", 0);		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap.ID);
		draw_cube();		

		// check and calls events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwTerminate();	
	delete(pbrShader);
	delete(quadShader);	
	delete(basicShader);

	for (int i = 0; i < PBRSets.size(); i++)
		delete(PBRSets[i]);

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
			string s = (printscreen_FBO)->Export("", ImageType::PNG, 2);
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

	if (data) // data != NULL
	{
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
		case 2: format = intFormat = GL_RG; break;
		case 1: format = intFormat = GL_RED; break;
		default: format = intFormat = GL_RGB; break;
		}

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);	// generates texture		
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		cerr << DC_WARNING " Could not load texture '" << path << "'\n";		
	}		
	stbi_image_free(data);
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
unsigned int load_hdr_image(const char* path, bool flip)
{
	stbi_set_flip_vertically_on_load(flip);
	int width, height, nrComponents;
	float* data = stbi_loadf(path, &width, &height, &nrComponents, 0);
	unsigned int hdrTexture = 0;
	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
		std::cout << DC_ERROR " load_hdr_texture(): Failed to load HDR image" << std::endl;

	return hdrTexture;
}
float lerp(float a, float b, float f)
{
	return a + f * (b - a);
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
		// Position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)0);		
		// Normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void*)(3 * sizeof(float)));
		// Texture coordinates
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

unsigned int sphereVAO = 0, sphereVBO = 0, indexCount;
void draw_sphere(void)
{
	if (sphereVAO == 0)
	{
		glGenVertexArrays(1, &sphereVAO);

		unsigned int vbo, ebo;
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;
		std::vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359;
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
				normals.push_back(glm::vec3(xPos, yPos, zPos));
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		indexCount = indices.size();

		std::vector<float> data;
		for (unsigned int i = 0; i < positions.size(); ++i)
		{
			data.push_back(positions[i].x);
			data.push_back(positions[i].y);
			data.push_back(positions[i].z);
			if (uv.size() > 0)
			{
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
			if (normals.size() > 0)
			{
				data.push_back(normals[i].x);
				data.push_back(normals[i].y);
				data.push_back(normals[i].z);
			}
		}
		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		float stride = (3 + 2 + 3) * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));		
	}

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

PBRTextureSet::PBRTextureSet(std::string path, unsigned int irradianceMap, unsigned int pefilterMap, unsigned int brdfLUT) {
	albedo = ao = metallic = normal = roughness = 0;
	albedo = load_texture((path + "albedo.png").c_str(), false, false);
	ao = load_texture((path + "ao.png").c_str(), false, false);
	metallic = load_texture((path + "metallic.png").c_str(), false, false);
	normal = load_texture((path + "normal.png").c_str(), false, false);
	roughness = load_texture((path + "roughness.png").c_str(), false, false);
	this->irradianceMap = irradianceMap;
	this->prefilterMap = pefilterMap;
	this->brdfLUT = brdfLUT;
}
void PBRTextureSet::Set(Shader* s) {
	s->Use();
	s->setInt("albedoMap", 0);
	s->setInt("aoMap", 1);
	s->setInt("metallicMap", 2);
	s->setInt("normalMap", 3);
	s->setInt("roughnessMap", 4);
	s->setInt("irradianceMap", 5);
	s->setInt("prefilterMap", 6);
	s->setInt("brdfLUT", 7);
}
void PBRTextureSet::Bind() {	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->albedo);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->ao);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, this->metallic);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, this->normal);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, this->roughness);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->irradianceMap);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->prefilterMap);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, this->brdfLUT);
}