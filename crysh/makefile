NY:clean all
CC=gcc
CFLAGS=-Werror -Wall -Wpedantic -g 
BIN=crysh
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
crysh:crysh.o decrypt.o handle.o
	$(CC) $(CFLAGS) -lc -lcrypto crysh.o decrypt.o handle.o -o $@
	
#srv2:srv2.o sckutil.o
#	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(BIN)
