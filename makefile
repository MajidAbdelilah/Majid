# Project: Majid Game Engine
# Makefile created by Red Panda C++ 2.22

CPP      = g++
CC       = gcc
OBJ      = obj/ufbx.o obj/main.o obj/mathc.o obj/renderer.o obj/image.o obj/model_loader.o
LINKOBJ  = obj/ufbx.o obj/main.o obj/mathc.o obj/renderer.o obj/image.o obj/model_loader.o
CLEANOBJ  =  obj/ufbx.o obj/main.o obj/mathc.o obj/renderer.o obj/image.o obj/model_loader.o "bin/Majid Game Engine"
LIBS     =  -L"/home/ain/projects/Majid_Game_Engine/lib" -lglfw -lm -lvulkan -lpthread -lpthread -lm -lvulkan -lglfw
INCS     =  -I"/home/ain/projects/Majid_Game_Engine/src" -I"/home/ain/projects/Majid_Game_Engine/lib/stb"  -I"/home/ain/projects/Majid Game Engine/lib/stb"
CXXINCS  =   -I"/home/ain/projects/Majid Game Engine/lib/stb"
BIN      = "bin/Majid Game Engine"
CXXFLAGS = $(CXXINCS)  -g3 -O1 -pipe -Wall -D__DEBUG__
CFLAGS   = $(INCS)  -std=c99 -g3 -O1 -pipe -Wall -D__DEBUG__
RM       = rm -rf

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	-$(RM) $(CLEANOBJ) > /dev/null 2>&1

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

obj/ufbx.o: src/ufbx.c src/ufbx.h
	$(CC) -c src/ufbx.c -o obj/ufbx.o $(CFLAGS) 

obj/main.o: src/main.c src/renderer.h
	$(CC) -c src/main.c -o obj/main.o $(CFLAGS) 

obj/mathc.o: src/mathc.c src/mathc.h
	$(CC) -c src/mathc.c -o obj/mathc.o $(CFLAGS) 

obj/renderer.o: src/renderer.c src/ufbx.h src/ufbx/sokol_gfx.h src/renderer_structs.h src/mathc.h src/image.h src/renderer.h src/ufbx/umath.h src/model_loader.h
	$(CC) -c src/renderer.c -o obj/renderer.o $(CFLAGS) 

obj/image.o: src/image.c src/image.h
	$(CC) -c src/image.c -o obj/image.o $(CFLAGS) 

obj/model_loader.o: src/model_loader.c src/ufbx.h src/ufbx/sokol_gfx.h src/renderer_structs.h src/mathc.h src/image.h src/ufbx/umath.h src/model_loader.h
	$(CC) -c src/model_loader.c -o obj/model_loader.o $(CFLAGS) 
