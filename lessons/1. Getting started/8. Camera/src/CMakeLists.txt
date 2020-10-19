add_library(camera Camera.cpp Camera.h)
add_library(shader Shader.cpp Shader.h)

include_directories(
	"${CMAKE_CURRENT_BINARY_DIR}"
	"${REPO_ROOT}/include"
)

set(REPO_ROOT "${CMAKE_SOURCE_DIR}/../../..")
set(PROJECT_DIR "${CMAKE_SOURCE_DIR}")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

configure_file(config.h.in config.h @ONLY)

add_executable(${PROJECT_NAME} 
	main.cpp	
	"${REPO_ROOT}/glad.c"
	"${REPO_ROOT}/stb_image.cpp"	
)