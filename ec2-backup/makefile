NY:clean all
CC=gcc
CFLAGS=-Werror -Wall
BIN=ec2-backup
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BIN):main.o util.o
	$(CC) $(CFLAGS) main.o util.o -o $@
clean:
	rm -f *.o $(BIN)
