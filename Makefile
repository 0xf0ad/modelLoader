CXX = g++
CC = gcc
CFLAGS = -c -g -Wall -o
LDFLAGS = -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp -g -o

default : all compile

bin :
	find bin -quit || mkdir bin

clean :
	rm bin/*  || echo "all the object files are deleted already"
	rm -d bin || echo "the bin directory is alredy deleted"
	rm load   || echo "the binary is deleted already"

all : stbi glad shader mesh model main

compile :
	$(CXX) bin/*.o $(LDFLAGS) load

main : bin
	$(CXX) main.cpp $(CFLAGS) bin/main.o

glad : bin
	$(CC) glad.c $(CFLAGS) bin/glad.o

shader : bin
	$(CXX) shader.cpp $(CFLAGS) bin/shader.o

stbi : bin
	$(CC) stb_image.c $(CFLAGS) bin/stb_image.o

model : bin
	$(CXX) model.cpp $(CFLAGS) bin/model.o

mesh : bin
	$(CXX) mesh.cpp $(CFLAGS) bin/mesh.o

install : all
	mv load /usr/bin/
	make clean
