project('3. Hello Triangle', ['cpp', 'c'], version : '1.0-0')

# include + libraries
include = include_directories('../../../include')
libsDir = [meson.current_source_dir() + '/../../../lib']

cxx = meson.get_compiler('cpp')
libs = [cxx.find_library('glfw3', dirs : libsDir, static : true),
	cxx.find_library('dl'),
	cxx.find_library('pthread')]
depend = [dependency('X11')]

# source files
sourceFiles = ['src/main.cpp',
	       '../../../glad.c',
	       'src/Camera.cpp',
	       'src/Shader.cpp']

# executable
main = executable('main', 
		  sourceFiles, 
		  include_directories : include, 
		  dependencies : [depend, libs],
		  objects : '../../../stb_image.o',
		  cpp_args : '-Wno-reorder')
test('HelloTriangle test', main)
