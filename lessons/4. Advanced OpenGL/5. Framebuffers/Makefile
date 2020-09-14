NAME := main
CXX=g++
DEBUG := 
CXX_FLAGS := $(DEBUG) -std=c++14
LIBS := -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -pthread -lXi -ldl -lm -lassimp
INCLUDE := ../../../include/
BIN := bin
SRC := src
INPUT = src/main.cpp ../../../glad.c

all: $(BIN)/$(NAME)

run:
	cd $(BIN) && ./$(NAME)

$(BIN)/$(NAME): $(SRC)/main.o $(SRC)/glad.o $(SRC)/Shader.o $(SRC)/stb_image.o $(SRC)/Camera.o $(SRC)/Framebuffer.o
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) $^ -o $@ $(LIBS)

.cpp.o:
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -c $< -o $@

.c.o:
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -c $< -o $@

$(SRC)/glad.o: ../../../glad.c
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -c $< -o $@

$(SRC)/stb_image.o: ../../../stb_image.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -c $< -o $@

# dependencies
$(SRC)/main.o: $(SRC)/main.cpp
$(SRC)/Shader.o: $(SRC)/Shader.cpp

clean:
	rm -rf $(SRC)/*.o	
	touch $(SRC)/*
