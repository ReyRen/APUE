#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)


//阻塞和非阻塞
int main(void )
{
	int pipefd[2];
	pid_t pid;
	if (pipe(pipefd) == -1 )	
	{
		printf("pipe() err..\n");	
		return -1;
	}
	pid = fork();
	if (pid == -1)
	{
		printf("fork err..\n");
		return -1;
	}
	
	if (pid == 0)
	{
		sleep(3);
		close(pipefd[0]);
		write(pipefd[1], "hello hello....", 6);
		close(pipefd[1]);
		printf("child .....quit\n");
	} 
	else if (pid > 0 )
	{
		int len = 0; 
		char buf[100] = {0};
		close(pipefd[1]);
		printf("begin read ...\n");
		len = read(pipefd[0], buf, 100);
		printf("begin after ...\n");
		printf("len:%d, buf:%s \n", len , buf);

		close(pipefd[0]);
	}

	wait(NULL);
	printf("parent ..quit\n");
	return 0;

}

