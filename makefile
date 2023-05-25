# Project: Majid Game Engine
# Makefile created by Red Panda C++ 2.22

CPP      = clang++
CC       = clang
OBJ      = obj/camera.o obj/renderer.o obj/main.o obj/model_loader.o obj/image.o obj/nodeTree.o
LINKOBJ  = obj/camera.o obj/renderer.o obj/main.o obj/model_loader.o obj/image.o obj/nodeTree.o
CLEANOBJ  =  obj/camera.o obj/renderer.o obj/main.o obj/model_loader.o obj/image.o obj/nodeTree.o "bin/Majid Game Engine"
LIBS     =  -m64 -lpthread -lm -lvulkan -lglfw  -L./lib/ -l:libufbx.a -L./lib/ -l:libmathc.a
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

obj/camera.o: src/camera.c src/ufbx/umath.h src/renderer.h src/image.h src/ufbx/sokol_gfx.h src/renderer_structs.h src/mathc.h src/camera.h src/ufbx.h
	$(CC) -c src/camera.c -o obj/camera.o $(CFLAGS) 

obj/renderer.o: src/renderer.c src/ufbx/umath.h src/renderer.h src/image.h src/ufbx/sokol_gfx.h src/renderer_structs.h src/model_loader.h src/mathc.h src/camera.h src/ufbx.h
	$(CC) -c src/renderer.c -o obj/renderer.o $(CFLAGS) 

obj/main.o: src/main.c src/ufbx/umath.h src/renderer.h src/image.h src/ufbx/sokol_gfx.h src/renderer_structs.h src/mathc.h src/camera.h src/ufbx.h
	$(CC) -c src/main.c -o obj/main.o $(CFLAGS) 

obj/model_loader.o: src/model_loader.c src/ufbx/umath.h src/image.h src/ufbx/sokol_gfx.h src/renderer_structs.h src/model_loader.h src/mathc.h src/ufbx.h
	$(CC) -c src/model_loader.c -o obj/model_loader.o $(CFLAGS) 

obj/image.o: src/image.c src/image.h
	$(CC) -c src/image.c -o obj/image.o $(CFLAGS) 

obj/nodeTree.o: src/nodeTree.c src/nodeTree.h src/xxhash_header.h src/mathc.h
	$(CC) -c src/nodeTree.c -o obj/nodeTree.o $(CFLAGS) 
