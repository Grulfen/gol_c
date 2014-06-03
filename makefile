CC = gcc
CFLAGS = -Wall -Wextra -W -pedantic -std=c99 -D_POSIX_C_SOURCE=199309L -fopenmp

ifeq ($(mode),release)
	CFLAGS += -O2
else
	CFLAGS += -g
endif

BIN = bin
SRC = src
LDFLAGS = -lncurses -fopenmp


OBJ = $(BIN)/game.o

EXE = game

all:$(EXE) 

$(BIN)/%.o : $(SRC)/%.c
	$(CC) -c $(CFLAGS) $? -o $@

game:   $(OBJ)
	$(CC) $^ $(LDFLAGS) -o $@

clean:
	rm -f $(EXE) $(BIN)/*.o
