set(REPO_ROOT "${CMAKE_SOURCE_DIR}/../../..")
set(PROJECT_DIR "${CMAKE_SOURCE_DIR}")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCE_INCLUDE "${SOURCE_DIR}/include")
set(BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

include_directories(
	"${CMAKE_CURRENT_BINARY_DIR}"
	"${REPO_ROOT}/include"
	"${SOURCE_INCLUDE}"
)

add_library(camera Camera.cpp include/Camera.h)
add_library(shader Shader.cpp include/Shader.h)
add_subdirectory("glml")
add_subdirectory("glfbo")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(DEBUG 1)
endif()


configure_file(config.h.in config.h @ONLY)

file(GLOB SHADERS
	"shaders/*.frag"
	"shaders/*.vert"
	"shaders/*.geom"
)

add_executable("${PROJECT_NAME}"
	main.cpp	
	"${REPO_ROOT}/glad.c"
	"${REPO_ROOT}/stb_image.cpp"
	"${REPO_ROOT}/stb_image_write.cpp"
	include/DebugColors.h
	"${BINARY_DIR}/config.h"
	"${SHADERS}"
)
source_group(TREE ${SOURCE_DIR} PREFIX "" FILES ${SHADERS})
