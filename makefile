# Project: Majid Game Engine
# Makefile created by Red Panda C++ 2.22

CPP      = clang++
CC       = clang
OBJ      = obj/io.o obj/renderer.o obj/model_loader.o obj/GUI.o obj/main.o obj/camera.o obj/vertex_index_optimizer.o obj/SIMD_check.o obj/nodeTree.o obj/image.o
LINKOBJ  = obj/io.o obj/renderer.o obj/model_loader.o obj/GUI.o obj/main.o obj/camera.o obj/vertex_index_optimizer.o obj/SIMD_check.o obj/nodeTree.o obj/image.o
CLEANOBJ  =  obj/io.o obj/renderer.o obj/model_loader.o obj/GUI.o obj/main.o obj/camera.o obj/vertex_index_optimizer.o obj/SIMD_check.o obj/nodeTree.o obj/image.o "bin/Majid Game Engine"
LIBS     =  -fprofile-generate -lstdc++ -lpthread -lm -lvulkan -lglfw  -L./lib/ -l:libufbx.a -L./lib/ -l:libmathc.a -L./lib/ -l:libturbojpeg.a  -L./lib/ -l:libmeshoptimizer.a
INCS     =   -I"/home/ain/projects/Majid Game Engine/lib" -I"/home/ain/projects/Majid Game Engine/lib/stb"
CXXINCS  =   -I"/home/ain/projects/Majid Game Engine/lib" -I"/home/ain/projects/Majid Game Engine/lib/stb"
BIN      = "bin/Majid Game Engine"
CXXFLAGS = $(CXXINCS)  -g3 -pipe -Wall -D__DEBUG__
CFLAGS   = $(INCS)  -g3 -pipe -Wall -fprofile-generate -D__DEBUG__
RM       = rm -rf

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	-$(RM) $(CLEANOBJ) > /dev/null 2>&1

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

obj/io.o: src/io.c src/io.h
	$(CC) -c src/io.c -o obj/io.o $(CFLAGS) 

obj/renderer.o: src/renderer.c src/io.h src/camera.h src/renderer.h src/ufbx/umath.h src/image.h src/model_loader.h src/vertex_index_optimizer.h src/renderer_structs.h src/ufbx/sokol_gfx.h
	$(CC) -c src/renderer.c -o obj/renderer.o $(CFLAGS) 

obj/model_loader.o: src/model_loader.c src/ufbx/umath.h src/model_loader.h src/renderer_structs.h src/ufbx/sokol_gfx.h
	$(CC) -c src/model_loader.c -o obj/model_loader.o $(CFLAGS) 

obj/GUI.o: src/GUI.c src/camera.h src/renderer.h src/ufbx/umath.h src/renderer_structs.h src/ufbx/sokol_gfx.h
	$(CC) -c src/GUI.c -o obj/GUI.o $(CFLAGS) 

obj/main.o: src/main.c src/camera.h src/renderer.h src/image.h
	$(CC) -c src/main.c -o obj/main.o $(CFLAGS) 

obj/camera.o: src/camera.c src/camera.h src/renderer.h
	$(CC) -c src/camera.c -o obj/camera.o $(CFLAGS) 

obj/vertex_index_optimizer.o: src/vertex_index_optimizer.c src/vertex_index_optimizer.h
	$(CC) -c src/vertex_index_optimizer.c -o obj/vertex_index_optimizer.o $(CFLAGS) 

obj/SIMD_check.o: src/SIMD_check.c src/SIMD_check.h
	$(CC) -c src/SIMD_check.c -o obj/SIMD_check.o $(CFLAGS) 

obj/nodeTree.o: src/nodeTree.c src/xxhash_header.h src/nodeTree.h
	$(CC) -c src/nodeTree.c -o obj/nodeTree.o $(CFLAGS) 

obj/image.o: src/image.c src/io.h src/image.h
	$(CC) -c src/image.c -o obj/image.o $(CFLAGS) 
