.PHONY:clean all
CC=gcc
CFLAGS=-Werror -Wall -g

BIN=signalstatus sigqueue
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
