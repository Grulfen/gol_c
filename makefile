CC = gcc
CFLAGS = -Wall -Wextra -W -pedantic -std=c99 -g -D_POSIX_C_SOURCE=199309L
BIN = bin
SRC = src
LDFLAGS = -lncurses


OBJ = $(BIN)/game.o

EXE = game

all:$(EXE) 

$(BIN)/%.o : $(SRC)/%.c
	$(CC) -c $(CFLAGS) $? $(LDFLAGS) -o $@

game:   $(OBJ)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f $(EXE) $(BIN)/*.o
