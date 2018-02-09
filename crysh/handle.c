#include "handle.h"

/*	FUNC DESCRIPTION:
 *		This func is used to handle the Decrypted data
 *		and split to token for how many different command
 *		have.
 *	PARAMETER:
 *		char * DecryptedData:
 *			Decrypted Data	
 *		char ** token:
 *			the storage to store the split result
 *	RETURN VALUE:
 *		err: RETURN_ERR_CODE
 *		success: token count 
 *
 * */
int handle_DecryptedData(char * DecryptedData, char ** token)
{
	int tokenCount = 0;
	
	if(DecryptedData == NULL)
	{
		perror("DecryptedData no content in handle_DecryptedData()");
		return RETURN_ERR_CODE;
	}
	token[tokenCount] = strtok(DecryptedData, TOKEN_FLAG);
	tokenCount++;
	while((token[tokenCount] = strtok(NULL, TOKEN_FLAG)) != NULL)
	{
		tokenCount++;
	}
	return tokenCount;
}

/*
 *	FUNC DESCRIPTION:
 *		This func is used to handle the tokened data
 *		and then look into each token's command and 
 *		split again for different arguments
 *	PARAMETER:
 *		char * token:
 *			token got from handle_token()
 *		char ** token_each
 *			the storage to store the split each token
 *	RETURN VALUE:
 *		err: RETURN_ERR_CODE
 *		success: count for each token arguments  
 *
 * */
int handle_token(char * token, char ** token_each)
{
	int token_each_count = 0;

	if(token == NULL)
	{
		perror("token no content in handle_token()");
		return RETURN_ERR_CODE;
	}
	token_each[token_each_count] = strtok(token, TOKEN_EACH_FLAG);
	token_each_count++;
	while((token_each[token_each_count] 
				= strtok(NULL, TOKEN_EACH_FLAG)) != NULL)
	{
		token_each_count++;	
	}
	return token_each_count;
}

/*
 *	FUNC DESCRIPTION:
 *		This func is used to check the file whether can assess
 *		into
 *	PARAMETERS:
 *		char * file:
 *			the file got from input or created
 *	RETURN VALUE:
 *		err: RETURN_ERR_CODE
 *		success: 0	
 * */
int file_handle(char * file)
{
	int mode;

	if(file == NULL)
	{
		perror("got file from handle_execute err");
		return RETURN_ERR_CODE;
	}
	mode = W_OK;

	if(access(file, mode) != 0)
	{
		perror("can not access to the file");
		return RETURN_ERR_CODE;
	}
	return 0;
}

/*
 *	FUNC DESCRIPTION:
 *		This func is center control func in handling
 *		decrypted data
 *	PARAMETER:
 *		char * DecryptedData:
 *			the decrypted data got from decrypt.c
 *	RETURN VALUE:
 *		err: RETURN_ERR_CODE
 *		success: 0
 * */
int handle_execute(char * DecryptedData)
{
	char * token[TOKEN_SIZE];
	char * token_each[TOKEN_SIZE];
	char * arguments[TOKEN_SIZE];
	char * file;
	int tokenCount;
	int token_each_count;
	int i = 0, j = 0, 
	    ret = 0, file_ret = 0, 
	    redirect = 0, redirect_err = 0, 
	    file_fd = 0;

	if(DecryptedData == NULL)
	{
		perror("Decrypted Data no content in handle_execute()");
		return RETURN_ERR_CODE;
	}
	bzero(arguments, sizeof(arguments));
	bzero(token, sizeof(token));
	bzero(token_each, sizeof(token_each));

	tokenCount = handle_DecryptedData(DecryptedData, token);
	if(tokenCount == 0 || tokenCount == RETURN_ERR_CODE)
	{
		perror("handle_DecryptedData err");
		return RETURN_ERR_CODE;	
	}
	for(i = 0; i < tokenCount; i++)	
	{
		if(i > 0)
		{
			token[i]++;
		}
		token_each_count = handle_token(token[i], token_each);
		if(token_each_count == RETURN_ERR_CODE)
		{
			return token_each_count;
		}
		if(token_each_count == 0)
		{
			perror("handle_token err");
			return RETURN_ERR_CODE;
		}
		for(j = 0; j < token_each_count; j++)
		{
			file_fd = 0;
			redirect = 0;
			redirect_err = 0;
			file = NULL;
			if(strstr(token_each[j], ERR_FLAG) != NULL)
			{
				if(strstr(token_each[j], APPEND_ERR) != NULL) 
				{
					file = token_each[j] + LEN_APPEND_ERR;
					file_fd = open(file, FLAG_APPEND, MODE);
					if(file_fd < 0)
					{
						perror("open file err");
						return RETURN_ERR_CODE;
					}
					file_ret = file_handle(file);
					if(file_ret == 0)
					{
						redirect_err = 1;
						break;
					}
					else
					{
						return file_ret; 
					}
				}
				if(strstr(token_each[j], TRUNC_ERR) != NULL)
				{
					file = token_each[j] + LEN_TRUNC_ERR;
					file_fd = open(file, FLAG_TRUNC, MODE);
					if(file_fd < 0)
					{
						perror("open file err");
						return RETURN_ERR_CODE;
					}
					file_ret = file_handle(file);
					if(file_ret == 0)
					{
						redirect_err = 1;
						break;
					}
					else
					{
						return file_ret;
					}
				}
				
			}
			else
			{
				if(strstr(token_each[j], APPEND) != NULL)
				{
					file = token_each[j] + LEN_APPEND;
					file_fd = open(file, FLAG_APPEND, MODE);
					if(file_fd < 0)
					{
						perror("open file err");	
						return RETURN_ERR_CODE;
					}
					file_ret = file_handle(file);
					if(file_ret == 0)
					{
						redirect = 1;
						break;
					}
					else
					{
						return file_ret;
					}
				}
				if(strstr(token_each[j], TRUNC) != NULL)
				{
					file = token_each[j] + LEN_TRUNC;
					file_fd = open(file, FLAG_TRUNC, MODE);
					if(file_fd < 0)
					{
						perror("open file err");
						return RETURN_ERR_CODE;
					}
					file_ret = file_handle(file);
					if(file_ret == 0)
					{
						redirect = 1;
						break;
					}
					else
					{
						return file_ret;
					}
				}
			}
			arguments[j] = token_each[j];
		}
		arguments[j] = (char *)NULL;
		ret = execute(arguments, file_fd, redirect, redirect_err);
		if(ret == RETURN_ERR_CODE)
		{
			break;
		}
	}
	return ret;
}

/*
 *	FUNC DESCRIPTION:
 *		This func is used to execute the command and output
 *		file or stdout
 *	PARAMETER:
 *		char ** arguments:
 *			execvp arguments
 *		int file_fd:
 *			the file descriptor is using redirection
 *		int redirect:
 *			check if the > or 2>
 *		int redirect_err:
 *			check if the >> or 2>>
 *	RETURN VALUE:
 *		err: RETURN_ERR_CODE
 *		success: 0
 *
 * */
int execute(char ** arguments, int file_fd, int redirect, int redirect_err) 
{
	int pipe_err[2];
	int pipe_out[2];
	char buf_out[BUFSIZE];
	char buf_err[BUFSIZE];
	int err_len = 0;
	int out_len = 0; 
	pid_t pid;

	if(arguments == NULL)
	{
		perror("arguments no content in execute()");
		return RETURN_ERR_CODE;
	}
	if(pipe(pipe_err) < 0 || pipe(pipe_out) < 0)
	{
		perror("pipe err in execute()");
		return RETURN_ERR_CODE;
	}
	pid = fork();
	if(pid < 0)
	{
		perror("fork err in execute()");
		return RETURN_ERR_CODE;
	}
	if(pid == 0)
	{
		dup2(pipe_out[1], STDOUT_FILENO);
		dup2(pipe_err[1], STDERR_FILENO);
		close(pipe_out[0]);
		close(pipe_err[0]);
		execvp(arguments[0], arguments);
	}
	else if(pid > 0)
	{
		dup2(pipe_err[0], STDIN_FILENO);
		dup2(pipe_out[0], STDIN_FILENO);
		close(pipe_err[1]);
		close(pipe_out[1]);

		do
		{
			bzero(buf_err, sizeof(buf_err));
			err_len = read(pipe_err[0], buf_err, sizeof(buf_err));
			if(err_len < 0)
			{
				perror("read err in execute()");
				close(pipe_err[0]);
				close(pipe_out[0]);
				wait(NULL);
				return RETURN_ERR_CODE;
			}
			if(redirect_err)
			{
				write(file_fd, buf_err, err_len);
			}
			else
			{
				if(err_len > 0)
				{
					close(pipe_err[0]);
					close(pipe_out[0]);
					wait(NULL);
					return RETURN_ERR_CODE;
				}
			}
		}while(err_len != 0);

		do
		{
			bzero(buf_out, sizeof(buf_out));
			out_len = read(pipe_out[0], buf_out, sizeof(buf_out));
			if(out_len < 0)
			{
				perror("read err in execute()");
				close(pipe_err[0]);
				close(pipe_out[0]);
				wait(NULL);
				return RETURN_ERR_CODE;
			}
			if(redirect)
			{
				write(file_fd, buf_out, out_len);
			}
			else 
			{	if(out_len > 0)
				{
					printf("%s", buf_out);	
				}
			}
		}while(out_len != 0);

		close(pipe_err[0]);
		close(pipe_out[0]);
		wait(NULL);

	}
	return 0;


}
