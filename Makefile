CXX = g++
CC = gcc
CFLAGS = -c -g -Wall
LDFLAGS = -lglfw -lGL -lrt -lm -lX11 -lpthread -lXrandr -lXi -ldl -lxcb -lXau -lXdmcp -lassimp -g
IMGUIDIR = ./gui

default : all compile

bin:
	find bin -quit || mkdir bin

clean:
	rm bin/*  || echo "all the object files are deleted already"
	rm -d bin || echo "the bin directory is alredy deleted"
	rm load   || echo "the binary is deleted already"

all:stbi glad imgui shader mesh bone model main

compile:
	$(CXX) bin/*.o $(LDFLAGS) -o load

main:bin
	$(CXX) main.cpp $(CFLAGS) -I$(IMGUIDIR) -I$(IMGUIDIR)/backends -o bin/main.o

glad:bin
	$(CC) glad.c $(CFLAGS) -o bin/glad.o

shader:bin
	$(CXX) shader.cpp $(CFLAGS) -o bin/shader.o

stbi:bin
	$(CC) stb_image.c $(CFLAGS) -o bin/stb_image.o

model:bin
	$(CXX) model.cpp $(CFLAGS) -o bin/model.o

mesh:bin
	$(CXX) mesh.cpp $(CFLAGS) -o bin/mesh.o

bone:bin
	$(CXX) bone.cpp $(CFLAGS) -o bin/bone.o

imgui:bin
	$(CXX) $(IMGUIDIR)/imgui.cpp $(CFLAGS) -o bin/imgui.o
	$(CXX) $(IMGUIDIR)/imgui_demo.cpp $(CFLAGS) -o bin/imgui_demo.o
	$(CXX) $(IMGUIDIR)/imgui_tables.cpp $(CFLAGS) -o bin/imgui_tables.o
	$(CXX) $(IMGUIDIR)/imgui_widgets.cpp $(CFLAGS) -o bin/imgui_widgets.o
	$(CXX) $(IMGUIDIR)/imgui_draw.cpp $(CFLAGS) -o bin/imgui_draw.o
	$(CXX) $(IMGUIDIR)/backends/imgui_impl_glfw.cpp $(CFLAGS) -I$(IMGUIDIR) -o bin/imgui_impl_glfw.o
	$(CXX) $(IMGUIDIR)/backends/imgui_impl_opengl3.cpp $(CFLAGS) -I$(IMGUIDIR) -o bin/imgui_impl_opengl3.o

realise:
	$(CXX) glad.c stb_image.c shader.cpp mesh.cpp model.cpp bone.cpp main.cpp $(LDFLAGS)0 -o load3

install:all
	mv load /usr/bin/
	make clean
