#ifndef CRYSH_HANDLE_
#define CRYSH_HANDLE_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#define FLAG_TRUNC  O_WRONLY | O_CREAT | O_TRUNC
#define FLAG_APPEND O_WRONLY | O_CREAT | O_APPEND
#define MODE S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH 
#define TOKEN_FLAG ";"
#define TOKEN_EACH_FLAG " "
#define TOKEN_SIZE 256
#define RETURN_ERR_CODE 128 
#define ERR_FLAG "2"
#define APPEND_ERR "2>>"
#define LEN_APPEND_ERR 3
#define TRUNC_ERR "2>"
#define LEN_TRUNC_ERR 2
#define APPEND ">>"
#define LEN_APPEND 2
#define TRUNC ">"
#define LEN_TRUNC 1
#define BUFSIZE 4096

int handle_DecryptedData(char * DecryptedData, char ** token);
int handle_token(char * token, char ** token_each);
int execute(char ** arguments, int file_fd, int redirect, int redirect_err);
int handle_execute(char * DecryptedData);

#endif

