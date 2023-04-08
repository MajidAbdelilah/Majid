MKDIR   := mkdir
RM   := rm
CC      := gcc
BIN     := ./bin
OBJ     := ./obj
LIB     := ./lib
INCLUDE := ./include
SRC     := ./src
SRCS    := $(wildcard $(SRC)/*.c)
OBJS    := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
BINARY     := Majid
CFLAGS := -I$(SRC) -I./lib/stb/   -msse -msse2 -mtune=native -std=gnu99 -Wall -g 
CDFLAGS  :=  -Wall -g
COFLAGS  :=  -Wall -Ofast
PGO_GEN_FLAGS = -fprofile-generate 
PGO_USE_FLAGS = -fprofile-use 
LDLIBS  :=-lpthread -lm -lvulkan -lglfw -L./lib/ -lmathc 
LDFLAGS := #-Ofast -Wall
flags := 
VERT_SHADER := $(SRC)/triangle.vert
FRAG_SHADER := $(SRC)/triangle.frag

#-Ofast -Wall

all: debug

debug: CFLAGS += $(CDFLAGS)
debug: LDFLAGS += $(CDFLAGS)
debug: $(BIN)/$(BINARY)

release: CFLAGS += $(COFLAGS)
release: LDFLAGS += $(COFLAGS)
release: $(BIN)/$(BINARY)

pgogen: CFLAGS += $(COFLAGS) $(PGO_GEN_FLAGS)
pgogen: LDFLAGS += $(COFLAGS) $(PGO_GEN_FLAGS)
pgogen: $(BIN)/$(BINARY)

pgouse: CFLAGS += $(COFLAGS) $(PGO_USE_FLAGS)
pgouse: LDFLAGS += $(COFLAGS) $(PGO_USE_FLAGS)
pgouse: $(BIN)/$(BINARY)



win_release: CC = x86_64-w64-mingw32-gcc
win_release: CFLAGS += -O0
win_release: LDFLAGS += -O0
win_release: LDLIBS  = -lm 
win_release: $(OBJS) | $(BIN)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BIN)/$(BINARY):  $(OBJS) | $(BIN) $(BIN)/triangle_frag_opt.spv $(BIN)/triangle_vert_opt.spv
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN) $(OBJ):
	$(MKDIR) $@

$(BIN)/triangle_vert_opt.spv: $(VERT_SHADER)
	glslc -O $< -o bin/triangle_vert.spv
	spirv-opt  --merge-return --inline-entry-points-exhaustive --eliminate-dead-functions --scalar-replacement --eliminate-local-single-block --eliminate-local-single-store --simplify-instructions --vector-dce --eliminate-dead-inserts --eliminate-dead-code-aggressive --eliminate-dead-branches --merge-blocks --eliminate-local-multi-store --simplify-instructions --vector-dce --eliminate-dead-inserts --redundancy-elimination --eliminate-dead-code-aggressive --strip-debug bin/triangle_vert.spv -o $@

$(BIN)/triangle_frag_opt.spv: $(FRAG_SHADER)
	glslc -O $< -o bin/triangle_frag.spv
	spirv-opt  --merge-return --inline-entry-points-exhaustive --eliminate-dead-functions --scalar-replacement --eliminate-local-single-block --eliminate-local-single-store --simplify-instructions --vector-dce --eliminate-dead-inserts --eliminate-dead-code-aggressive --eliminate-dead-branches --merge-blocks --eliminate-local-multi-store --simplify-instructions --vector-dce --eliminate-dead-inserts --redundancy-elimination --eliminate-dead-code-aggressive --strip-debug bin/triangle_frag.spv -o $@



run: $(BIN)/$(BINARY)
	cd $(BIN) && ./$(BINARY) $(flags)

clean:
	$(RM) $(OBJ)/*.o $(BIN)/$(BINARY) $(BIN)/*.spv

ddd: debug
	cd $(BIN) && ddd  ./$(BINARY) $(flags)
valgrind: debug
	cd $(BIN) && valgrind  ./$(BINARY) $(flags)
