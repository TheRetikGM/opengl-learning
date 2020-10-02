project('3. Hello Triangle', ['cpp', 'c'], version : '1.0-0')

libsDir = [meson.current_source_dir() + '/../../../lib']

cxx = meson.get_compiler('cpp')
libs = [cxx.find_library('glfw3', dirs : libsDir, static : true),
	cxx.find_library('dl'),
	cxx.find_library('pthread')]

depend = [dependency('X11')]
sourceFiles = ['src/main.cpp',
	       '../../../glad.c']

include = include_directories('../../../include')

main = executable('main', sourceFiles, include_directories : include, dependencies : [depend, libs])
test('HelloTriangle test', main)
