# Project: Majid Game Engine
# Makefile created by Red Panda C++ 2.21

CPP      = g++
CC       = gcc
OBJ      = obj/main.o obj/model_loader.o obj/mathc.o obj/image.o obj/renderer.o obj/ufbx.o
LINKOBJ  = obj/main.o obj/model_loader.o obj/mathc.o obj/image.o obj/renderer.o obj/ufbx.o
CLEANOBJ  =  obj/main.o obj/model_loader.o obj/mathc.o obj/image.o obj/renderer.o obj/ufbx.o "bin/Majid Game Engine"
LIBS     =  -pg -s -lpthread -lm -lvulkan -lglfw
INCS     =   -I"/home/ain/projects/Majid Game Engine/lib/stb"
CXXINCS  =   -I"/home/ain/projects/Majid Game Engine/lib/stb"
BIN      = "bin/Majid Game Engine"
CXXFLAGS = $(CXXINCS)  -g3 -Ofast -pipe -D__DEBUG__
CFLAGS   = $(INCS)  -std=c99 -g3 -Ofast -pipe -D__DEBUG__
RM       = rm -rf

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	-$(RM) $(CLEANOBJ) > /dev/null 2>&1

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

obj/main.o: src/main.c
	$(CC) -c src/main.c -o obj/main.o $(CFLAGS) 

obj/model_loader.o: src/model_loader.c
	$(CC) -c src/model_loader.c -o obj/model_loader.o $(CFLAGS) 

obj/mathc.o: src/mathc.c
	$(CC) -c src/mathc.c -o obj/mathc.o $(CFLAGS) 

obj/image.o: src/image.c
	$(CC) -c src/image.c -o obj/image.o $(CFLAGS) 

obj/renderer.o: src/renderer.c
	$(CC) -c src/renderer.c -o obj/renderer.o $(CFLAGS) 

obj/ufbx.o: src/ufbx.c
	$(CC) -c src/ufbx.c -o obj/ufbx.o $(CFLAGS) 
