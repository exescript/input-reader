CC=gcc
CFLAGS=-Wall --std=c11

BIN=./inreader
SRC=main.c debug.c runtime.c
HDR=runtime.h events.h debug.h

all: $(BIN)

$(BIN): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

clean:
	rm *.o
