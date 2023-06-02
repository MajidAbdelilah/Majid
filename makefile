# Project: Majid Game Engine
# Makefile created by Red Panda C++ 2.22

CPP      = clang++
CC       = clang
OBJ      = obj/model_loader.o obj/main.o obj/renderer.o obj/nodeTree.o obj/image.o obj/io.o obj/camera.o
LINKOBJ  = obj/model_loader.o obj/main.o obj/renderer.o obj/nodeTree.o obj/image.o obj/io.o obj/camera.o
CLEANOBJ  =  obj/model_loader.o obj/main.o obj/renderer.o obj/nodeTree.o obj/image.o obj/io.o obj/camera.o "bin/Majid Game Engine"
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

obj/model_loader.o: src/model_loader.c src/model_loader.h src/ufbx/sokol_gfx.h src/renderer_structs.h src/ufbx/umath.h
	$(CC) -c src/model_loader.c -o obj/model_loader.o $(CFLAGS) 

obj/main.o: src/main.c src/renderer.h src/camera.h src/image.h lib/stb/stb_image.h
	$(CC) -c src/main.c -o obj/main.o $(CFLAGS) 

obj/renderer.o: src/renderer.c src/renderer.h src/camera.h src/model_loader.h src/ufbx/sokol_gfx.h src/image.h src/renderer_structs.h lib/stb/stb_image.h src/io.h src/ufbx/umath.h
	$(CC) -c src/renderer.c -o obj/renderer.o $(CFLAGS) 

obj/nodeTree.o: src/nodeTree.c src/nodeTree.h src/xxhash_header.h
	$(CC) -c src/nodeTree.c -o obj/nodeTree.o $(CFLAGS) 

obj/image.o: src/image.c src/image.h lib/stb/stb_image.h src/io.h
	$(CC) -c src/image.c -o obj/image.o $(CFLAGS) 

obj/io.o: src/io.c src/io.h
	$(CC) -c src/io.c -o obj/io.o $(CFLAGS) 

obj/camera.o: src/camera.c src/renderer.h src/camera.h
	$(CC) -c src/camera.c -o obj/camera.o $(CFLAGS) 
