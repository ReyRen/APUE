.PHONY:clean all
CC=gcc
CFLAGS=-Werror -Wall -g

BIN=basefork multifork ZomOrProc file execlp daemon
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
