gdown --id 1vwYYukhdkL5xe_5x8M-URTUYcAPzmLeG -O libs.zip
gdown --id 122f9x8NI6Ujn459-S0f8SnoXCW6mqpzQ -O models.zip
gdown --id 14VkfsiVsPu3t7SZuMo1usfRq3RDLYcYY -O textures.zip

mkdir lib
mkdir models
mkdir textures

tar -xf libs.zip -C ./
tar -xf models.zip -C ./
tar -xf textures.zip -C ./

del libs.zip
del models.zip
del textures.zip