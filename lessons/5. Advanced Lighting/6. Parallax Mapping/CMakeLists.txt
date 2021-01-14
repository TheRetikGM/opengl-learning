cmake_minimum_required(VERSION 3.13.4)

project("main")
set(CMAKE_CXX_STANDARD 11)

add_subdirectory("src")

target_include_directories("${PROJECT_NAME}"
	PUBLIC "${CMAKE_SOURCE_DIR}/../../../include"	
)
target_link_directories("${PROJECT_NAME}"
	PRIVATE "${CMAKE_SOURCE_DIR}/../../../lib"
)

target_link_libraries("${PROJECT_NAME}"
	camera
	shader
	framebuffer
	glml
)
if (MSVC)
	set(AssimpLibs
		debug Debug/assimp-vc142-mtd optimized Release/assimp-vc142-mt
		debug Debug/IrrXMLd optimized Release/IrrXML
		debug Debug/zlibstaticd optimized Release/zlibstatic)
	set (glfw3Lib debug Debug/glfw3d optimized Release/glfw3)
	
	target_link_libraries("${PROJECT_NAME}"
		opengl32
		${AssimpLibs}
		${glfw3Lib}
	)
elseif (UNIX)
	target_link_libraries("${PROJECT_NAME}"
		glfw3
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
