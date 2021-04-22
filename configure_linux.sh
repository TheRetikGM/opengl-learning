#!/bin/bash

# This configuration is made for linux only!

gdown --id 122f9x8NI6Ujn459-S0f8SnoXCW6mqpzQ -O models.zip
gdown --id 14VkfsiVsPu3t7SZuMo1usfRq3RDLYcYY -O textures.zip

mkdir models textures

unzip models.zip -d models
unzip textures.zip -d textures

rm textures.zip
rm models.zip
