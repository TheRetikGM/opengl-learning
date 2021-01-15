#!/bin/bash

# This configuration is made for linux only!

lib_arch=lib_arch.tar.gz

gdown --id 1ZpoCCa0VFwJoJOwLVYEop8_uKQO2XRCh -O $lib_arch
gdown --id 1otyqcT-TohaHtv-rzguJn3lYCBBGXRVG -O models.zip

mkdir lib models

tar -xzvf $lib_arch -C lib
unzip models.zip -d models

rm $lib_arch
rm models.zip
