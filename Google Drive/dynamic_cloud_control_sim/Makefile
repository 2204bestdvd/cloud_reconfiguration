EXE = main

SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CC = clang++

CPPFLAGS += -Iinclude
CFLAGS += -std=c++11

.PHONY: all clean

all: $(EXE) 

$(EXE): $(OBJ)
	$(CC) -std=c++11 $^ -o ./test

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

