# Project: Majid Editor
# Makefile created by Red Panda C++ 2.22

CPP      = clang++
CC       = clang
OBJ      = ../../../obj/nuk_imp.o ../../../obj/main.o
LINKOBJ  = ../../../obj/nuk_imp.o ../../../obj/main.o
CLEANOBJ  =  ../../../obj/nuk_imp.o ../../../obj/main.o "../../../bin/Majid Editor"
LIBS     =  -lm -lGL -lm -lGLU -lGLEW -lglfw
INCS     =   -I"/home/ain/projects/Majid Game Engine/lib"
CXXINCS  =   -I"/home/ain/projects/Majid Game Engine/lib"
BIN      = "../../../bin/Majid Editor"
CXXFLAGS = $(CXXINCS)  -g3 -Og -pipe -Wall -D__DEBUG__
CFLAGS   = $(INCS)  -std=gnu99 -g3 -Og -pipe -Wall -D__DEBUG__
RM       = rm -rf

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	-$(RM) $(CLEANOBJ) > /dev/null 2>&1

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

../../../obj/nuk_imp.o: nuk_imp.c
	$(CC) -c nuk_imp.c -o ../../../obj/nuk_imp.o $(CFLAGS) 

../../../obj/main.o: main.c
	$(CC) -c main.c -o ../../../obj/main.o $(CFLAGS) 
