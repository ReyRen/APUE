.PHONY:clean all
CC=gcc
CFLAGS=-Werror -Wall -g
BIN=mulfork 


mulfork: mulfork.o myipc_sem.o myipc_shm.o
	$(CC) $(CFLAGS) $^ -o $@

all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
