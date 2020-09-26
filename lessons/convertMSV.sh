#!/bin/bash

dir=$1

mkdir src bin
mkdir src/shaders

echo $dir
cd "$dir"
mv *.h ../src/
mv *.cpp ../src/
mv *.c ../src/
mv *.hpp ../src/
mv *.vert ../src/shaders
mv *.frag ../src/shaders
mv *.geom ../src/shaders
cd ..
rm -r "$dir"
rm -r x64
rm -r Debug
rm -r "$dir.sln"
rm -rf .vs
