# Project: Majid Game Engine
# Makefile created by Red Panda C++ 2.22

CPP      = clang++
CC       = clang
OBJ      = obj/renderer.o obj/camera.o obj/main.o obj/image.o obj/model_loader.o obj/io.o obj/nodeTree.o
LINKOBJ  = obj/renderer.o obj/camera.o obj/main.o obj/image.o obj/model_loader.o obj/io.o obj/nodeTree.o
CLEANOBJ  =  obj/renderer.o obj/camera.o obj/main.o obj/image.o obj/model_loader.o obj/io.o obj/nodeTree.o "bin/Majid Game Engine"
LIBS     =  -m64 -lpthread -lm -lvulkan -lglfw  -L./lib/ -l:libufbx.a -L./lib/ -l:libmathc.a -L./lib/ -l:libturbojpeg.a
INCS     =   -I"/home/ain/projects/Majid Game Engine/lib" -I"/home/ain/projects/Majid Game Engine/lib/stb"
CXXINCS  =   -I"/home/ain/projects/Majid Game Engine/lib" -I"/home/ain/projects/Majid Game Engine/lib/stb"
BIN      = "bin/Majid Game Engine"
CXXFLAGS = $(CXXINCS)  -g3 -Og -pipe -Wall -D__DEBUG__
CFLAGS   = $(INCS)  -std=gnu99 -g3 -Og -pipe -Wall -D__DEBUG__
RM       = rm -rf

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	-$(RM) $(CLEANOBJ) > /dev/null 2>&1

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

obj/renderer.o: src/renderer.c src/ufbx/umath.h src/renderer_structs.h src/model_loader.h src/ufbx/sokol_gfx.h lib/stb/stb_image.h src/camera.h src/image.h src/renderer.h src/io.h
	$(CC) -c src/renderer.c -o obj/renderer.o $(CFLAGS) 

obj/camera.o: src/camera.c src/camera.h src/renderer.h
	$(CC) -c src/camera.c -o obj/camera.o $(CFLAGS) 

obj/main.o: src/main.c lib/stb/stb_image.h src/camera.h src/image.h src/renderer.h
	$(CC) -c src/main.c -o obj/main.o $(CFLAGS) 

obj/image.o: src/image.c lib/stb/stb_image.h src/image.h src/io.h
	$(CC) -c src/image.c -o obj/image.o $(CFLAGS) 

obj/model_loader.o: src/model_loader.c src/ufbx/umath.h src/renderer_structs.h src/model_loader.h src/ufbx/sokol_gfx.h
	$(CC) -c src/model_loader.c -o obj/model_loader.o $(CFLAGS) 

obj/io.o: src/io.c src/io.h
	$(CC) -c src/io.c -o obj/io.o $(CFLAGS) 

obj/nodeTree.o: src/nodeTree.c src/xxhash_header.h src/nodeTree.h
	$(CC) -c src/nodeTree.c -o obj/nodeTree.o $(CFLAGS) 
