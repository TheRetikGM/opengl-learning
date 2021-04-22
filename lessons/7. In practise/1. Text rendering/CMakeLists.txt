cmake_minimum_required(VERSION 3.13.4)

project("TextRendering")
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
if (NOT DEFINED CMAKE_BUILD_TYPE)
	message(STATUS "Setting build type to 'Debug' as none was specified.")
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS  ${CMAKE_CONFIGURATION_TYPES})
endif()

set(CMAKE_DEBUG_POSTFIX "d")

add_subdirectory("src")

target_include_directories("${PROJECT_NAME}"
	PUBLIC "${CMAKE_SOURCE_DIR}/../../../../include"	
)

target_link_directories("${PROJECT_NAME}"
	PRIVATE "${CMAKE_SOURCE_DIR}/../../../lib"
	#	PRIVATE "${CMAKE_SOURCE_DIR}/../../../lib/Release"
	#	PRIVATE "${CMAKE_SOURCE_DIR}/../../../lib/Debug"
)

target_link_libraries("${PROJECT_NAME}"
	camera
	shader	
	glfbo
	glml
)

if (MSVC)
	set(AssimpLibs
		debug Debug/assimp-vc142-mtd optimized Release/assimp-vc142-mt
		debug Debug/IrrXMLd optimized Release/IrrXML
		debug Debug/zlibstaticd optimized Release/zlibstatic)
	set (glfw3Lib debug Debug/glfw3d optimized Release/glfw3)
	set (FreeTypeLib debug Debug/freetyped optimized Release/freetype)
	
	target_link_libraries("${PROJECT_NAME}"
		opengl32
		${AssimpLibs}
		${glfw3Lib}
		${FreeTypeLib}
	)
	install(TARGETS "${PROJECT_NAME}" DESTINATION bin)	
	include(CPack)				
				
elseif (UNIX)
	set(ASSIMP
		debug assimpd optimized assimp
		debug IrrXMLd optimized IrrXML
		debug zlibstaticd optimized zlibstatic)
	set(GLFW debug glfw3d optimized glfw3)
	target_link_libraries("${PROJECT_NAME}"
		glfw
		assimp
		dl
		m
		Xi
		pthread
		Xrandr
		X11
		GL
		GLU
	)
endif()
