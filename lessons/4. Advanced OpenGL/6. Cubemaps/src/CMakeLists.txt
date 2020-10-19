add_library(camera Camera.cpp Camera.h)
add_library(shader Shader.cpp Shader.h)
add_library(glml Model.cpp Model.h Mesh.cpp Mesh.h)
add_library(framebuffer Framebuffer.cpp Framebuffer.h)

set(REPO_ROOT "${CMAKE_SOURCE_DIR}/../../..")
set(PROJECT_DIR "${CMAKE_SOURCE_DIR}")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(
	"${CMAKE_CURRENT_BINARY_DIR}"
	"${REPO_ROOT}/include"
)

configure_file(config.h.in config.h @ONLY)

add_executable(${PROJECT_NAME} 
	main.cpp	
	"${REPO_ROOT}/glad.c"
	"${REPO_ROOT}/stb_image.cpp"
	"${REPO_ROOT}/stb_image_write.cpp"	
	shaders/quadShader.vert
	shaders/quadShader.frag
	shaders/shader.vert
	shaders/shader.frag
	shaders/skyboxShader.frag
	shaders/skyboxShader.vert
)
