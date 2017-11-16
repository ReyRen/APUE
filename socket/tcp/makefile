.PHONY:clean all
CC=gcc
CFLAGS=-Werror -Wall -g

BIN=ipbase srv clt srvmultifork p2pclt p2psrv
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
