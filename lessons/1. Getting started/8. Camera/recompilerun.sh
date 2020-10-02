#!/bin/bash

if [ ! -d "build" ]; then meson build; fi

cd build
meson compile
meson test
cd ..
