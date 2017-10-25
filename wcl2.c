#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_COMMAND 1024
#define STDERR 8
#define STDOUT 8

int main(int argc, char *argv[]) 
{
	int pipefd[2];
	pid_t pid;
	int i,j,errnum = 0;
	char *buf[MAX_COMMAND];
	FILE *fp;
	ssize_t nread;
	char a[2];	

	if(argc < 2)
	{
		perror("usage ./a.out <cmd>");
		return 1;
	}
	if(pipe(pipefd) < 0)
	{
		perror("pipe error");
		return 1;
	}
	if((pid = fork()) < 0)
	{
		perror("fork error");
		return 1;
	}
	else if(pid > 0)
	{
		close(pipefd[1]);
		if(dup2(pipefd[0], STDIN_FILENO) != STDIN_FILENO)
		{
			perror("dup2 error to stdin");
			return 1;
		}
		close(pipefd[0]);
		if(write(STDOUT_FILENO, "stderr: ", STDERR) != STDERR)
		{
			perror("write error");
			return 1;
		}
		if((nread = read(0, a, strlen(a) + 1)) == -1)
		{
			perror("read error");
			return 1;
		}
		if(write(STDOUT_FILENO, "\n", 2) != 2)
		{
			perror("write error");
			return 1;
		}
		if(write(STDOUT_FILENO, "stdout: ", STDOUT) != STDOUT)
		{
			perror("write error");
			return 1;
		}
		execlp("wc", "wc", "-l", NULL);
		close(pipefd[0]);
	}
	else
	{
		close(pipefd[0]);
		if(dup2(pipefd[1], STDOUT_FILENO) != STDOUT_FILENO)
		{
			perror("dup2 error to stdout");
			return 1;
		}
		close(pipefd[1]);
		if(argc == 2)
		{
			if(write(0, &errnum, 1) != 1)
			{
				perror("write error");
				return 1;
			}
			execlp(argv[1], argv[1], NULL);
		}
		else
		{
			j = 0;
			errnum = 0;
			for(i = 1; i < argc; i++)
			{
				if((i > 2) && (argv[i][0] != '-') && (fp = fopen(argv[i], "r")) == NULL)
				{
					errnum++;
					continue;
				}
				buf[j] = argv[i];
				j++;
			}	
			sprintf(a, "%d", errnum);
			if(write(0, a, strlen(a) + 1) != strlen(a) + 1)
			{
				perror("write error");
				return 1;
			}
			buf[j] = NULL;				
			execvp(argv[1], buf);

		}
	}
	return 0;
}
