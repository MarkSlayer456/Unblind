CC=gcc
CFLAGS=-lncurses
DEPS = unblind.h
OBJ = unblind2.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

unblind: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)