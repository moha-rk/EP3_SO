COMPILER = gcc

all: ep3

SRC_DIR = ./src
LIBS_DIR = ./lib
SIM_FLAGS = -Wall -I$(LIBS_DIR) -lreadline

LIBS_NAMES = commands.h filesystem.h
LIBS = $(patsubst %,$(LIBS_DIR)/%,$(LIBS_NAMES))

OBJ_NAMES = prompt.o filesystem.o commands.o
OBJ = $(patsubst %,$(SRC_DIR)/%,$(OBJ_NAMES))

$(SRC_DIR)/%.o: %.c $(LIBS)
	$(COMPILER) -c -o $@ $< $(SIM_FLAGS)

ep3: $(OBJ)
	$(COMPILER) -o $@ $^ $(SIM_FLAGS)

clean:
	rm -f $(shell find . -name '*.o' -or -name 'ep3')