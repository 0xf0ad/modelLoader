CFLAGS = -c -g -Wall
LDFLAGS = -lglfw -lGL -lrt -lm -lX11 -lpthread -lXrandr -lXi -ldl -lxcb -lXau -lXdmcp -lassimp -g
IMGUIDIR = ./gui
BUILDDIR = ./bin

default : all link

$(BUILDDIR):
	find bin -quit || mkdir bin

clean:
	rm $(BUILDDIR)/*  || echo "all the object files are deleted already"
	rm -d $(BUILDDIR) || echo "the bin directory is alredy deleted"
	rm ./load         || echo "the binary is deleted already"

all:$(BUILDDIR)/stbi.o $(BUILDDIR)/glad.o imgui $(BUILDDIR)/shader.o $(BUILDDIR)/mesh.o $(BUILDDIR)/model.o $(BUILDDIR)/bone.o $(BUILDDIR)/main.o

link: $(BUILDDIR)/*.o
	$(CXX) $(BUILDDIR)/*.o $(LDFLAGS) -o load

$(BUILDDIR)/main.o:src/main.cpp $(BUILDDIR)
	$(CXX) src/main.cpp $(CFLAGS) -I$(IMGUIDIR) -I$(IMGUIDIR)/backends -o $(BUILDDIR)/main.o

$(BUILDDIR)/glad.o:src/glad.c $(BUILDDIR)
	$(CC) src/glad.c $(CFLAGS) -o $(BUILDDIR)/glad.o

$(BUILDDIR)/shader.o:src/shader.cpp headers/shader.h $(BUILDDIR)
	$(CXX) src/shader.cpp $(CFLAGS) -o $(BUILDDIR)/shader.o

$(BUILDDIR)/stbi.o:src/stb_image.c headers/libs/stb_image.h $(BUILDDIR)
	$(CC) src/stb_image.c $(CFLAGS) -o $(BUILDDIR)/stbi.o

$(BUILDDIR)/model.o:src/model.cpp headers/model.h $(BUILDDIR)
	$(CXX) src/model.cpp $(CFLAGS) -o $(BUILDDIR)/model.o

$(BUILDDIR)/mesh.o:src/mesh.cpp headers/mesh.h $(BUILDDIR)
	$(CXX) src/mesh.cpp $(CFLAGS) -o $(BUILDDIR)/mesh.o

$(BUILDDIR)/bone.o:src/bone.cpp headers/bone.h $(BUILDDIR)
	$(CXX) src/bone.cpp $(CFLAGS) -o $(BUILDDIR)/bone.o

imgui:$(IMGUIDIR)/*.cpp $(IMGUIDIR)/*.h $(IMGUIDIR)/backends/*.cpp $(IMGUIDIR)/backends/*.h $(BUILDDIR)
	$(CXX) $(IMGUIDIR)/imgui.cpp $(CFLAGS) -o $(BUILDDIR)/imgui.o
	$(CXX) $(IMGUIDIR)/imgui_demo.cpp $(CFLAGS) -o $(BUILDDIR)/imgui_demo.o
	$(CXX) $(IMGUIDIR)/imgui_tables.cpp $(CFLAGS) -o $(BUILDDIR)/imgui_tables.o
	$(CXX) $(IMGUIDIR)/imgui_widgets.cpp $(CFLAGS) -o $(BUILDDIR)/imgui_widgets.o
	$(CXX) $(IMGUIDIR)/imgui_draw.cpp $(CFLAGS) -o $(BUILDDIR)/imgui_draw.o
	$(CXX) $(IMGUIDIR)/backends/imgui_impl_glfw.cpp $(CFLAGS) -I$(IMGUIDIR) -o $(BUILDDIR)/imgui_impl_glfw.o
	$(CXX) $(IMGUIDIR)/backends/imgui_impl_opengl3.cpp $(CFLAGS) -I$(IMGUIDIR) -o $(BUILDDIR)/imgui_impl_opengl3.o

realise:
	$(CXX) glad.c stb_image.c shader.cpp mesh.cpp model.cpp bone.cpp main.cpp $(LDFLAGS)0 -o load3

install:all
	mv load /usr/bin/
	make clean
