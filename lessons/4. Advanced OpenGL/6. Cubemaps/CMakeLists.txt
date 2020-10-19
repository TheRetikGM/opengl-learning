cmake_minimum_required(VERSION 3.13.4)

project("main")

add_subdirectory("src")

target_include_directories("${PROJECT_NAME}"
	PUBLIC "${CMAKE_SOURCE_DIR}/../../../include"	
)
target_link_directories("${PROJECT_NAME}"
	PRIVATE "${CMAKE_SOURCE_DIR}/../../../lib"
)
target_link_libraries("${PROJECT_NAME}"
	glfw3
	camera
	shader
	framebuffer
	glml
)
if (MSVC)
	target_link_libraries("${PROJECT_NAME}"
		opengl32		
		assimp-vc142-mt
		IrrXML
		zlibstatic
	)
elseif (UNIX)
	target_link_libraries("${PROJECT_NAME}"
		dl
		m
		Xi
		pthread
		Xrandr
		X11
		GL
		GLU
		assimp
	)
endif()
