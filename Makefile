default : all compile

bin :
	find bin -quit || mkdir bin

clean :
	rm bin/* || echo "all the object files are deleted already"
	rm bin   || echo "the bin directory is alredy deleted"
	rm load  || echo "the binary is deleted already"

all : stbi glad shader mesh model main

compile :
	g++ bin/*.o -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lassimp -g -o load

main : bin
	g++ -c main.cpp -lassimp -g -Wall -o bin/main.o

glad : bin
	gcc -c glad.c -Wall -g -o bin/glad.o

shader : bin
	g++ -c shader.cpp -g -Wall -o bin/shader.o

stbi : bin
	gcc -c stb_image.c -g -o bin/stb_image.o

model : bin
	g++ -c model.cpp -Wall -g -lassimp -o bin/model.o

mesh : bin
	g++ -c mesh.cpp -Wall -g -o bin/mesh.o

install : all
	mv load /usr/bin/
	make clean
