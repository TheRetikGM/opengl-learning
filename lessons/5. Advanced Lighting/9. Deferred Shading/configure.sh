#!/bin/bash

if [[ $1 == "-d" ]]; then
	echo "Debug configuration"
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
else
	echo "Release configuration"
	cmake -S . -B build
fi
