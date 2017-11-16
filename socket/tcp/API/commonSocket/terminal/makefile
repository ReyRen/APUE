NY:clean all
CC=gcc
CFLAGS=-Werror -Wall -g
BIN=server
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
server:server.o commonsocketT.o
	$(CC) $(CFLAGS) $^ -o $@
	
#srv2:srv2.o sckutil.o
#	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(BIN)
