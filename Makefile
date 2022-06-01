default: all compile

clean:
	rm bin/*
	rm load

all: stbi glad shader mesh model main

compile:
	g++ bin/*.o -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp -g -o load

main:
	g++ -c main.cpp -lassimp -g -Wall -o bin/main.o

glad:
	gcc -c glad.c -Wall -g -o bin/glad.o

shader:
	g++ -c shader.cpp -g -Wall -o bin/shader.o

stbi:
	gcc -c stb_image.c -g -o bin/stb_image.o

model:
	g++ -c model.cpp -Wall -g -lassimp -o bin/model.o

mesh:
	g++ -c mesh.cpp -Wall -g -o bin/mesh.o

install: all
	mv load /usr/bin/
	make clean
