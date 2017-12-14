#include "decrypt.h"
#include "handle.h"

#define BUFSIZE_MAIN 1024 
#define SALTED "Salted"

int main(int argc, char * argv[])
{
	char in_buf[BUFSIZE_MAIN];
	int len;
	int real_bytes_command;
	char * decryptedData; 
	char handle_buf[BUFSIZE_MAIN];
	char command_buf[BUFSIZE_MAIN];
	char * token[TOKEN_SIZE];
	int ret;

	bzero(in_buf, sizeof(in_buf));
	bzero(command_buf, sizeof(command_buf));
	bzero(handle_buf, sizeof(handle_buf));
	bzero(token, sizeof(token));
	 

	if(argc != 1)
	{
		printf("crysh usage: echo \"[command]\" | openssl enc -aes-256-cbc -md sha1 | ./crysh");
	}
	len = read(STDIN_FILENO,in_buf, sizeof(in_buf));
	if(len == -1)
	{
		perror("read err from stdin");
		return RETURN_ERR_CODE;
	}
	if(len == 0)
	{
		perror("There is nothing got from stdin");
		return RETURN_ERR_CODE;
	}
	if(strstr(in_buf, SALTED) == NULL)
	{
		printf("crysh: Unable to decrypt the input\n")	;
		return RETURN_ERR_CODE;
	}
	decryptedData =  decry(in_buf, &real_bytes_command, len);
	strncpy(handle_buf, decryptedData, real_bytes_command);
	decryptedData = handle_buf;

	if(decryptedData == NULL)	
	{
		return RETURN_ERR_CODE;
	}		
	ret = handle_execute(decryptedData);

	return ret;
}
