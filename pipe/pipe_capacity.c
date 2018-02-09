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
//测试管道容量

#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)


void myhandle(int sig)
{
	printf("recv sig:%d \n", sig);
}


int main(void )
{
	int pipefd[2];
	pid_t pid;
	
	//注册管道处理函数
	signal(SIGPIPE, myhandle);
	
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
		int count = 0;
		int ret = 0;
		close(pipefd[0]);
		
		
		//写端变成非阻塞
		int flags = fcntl(pipefd[1], F_GETFL);
		flags = flags | O_NONBLOCK;
		ret = fcntl(pipefd[1], F_SETFL, flags);
		if (ret == -1)
		{
			printf("fcntl err.\n");
			exit(0);
		}
		
		while(1)
		{
			ret = write(pipefd[1] , "a", 1);	
			if (ret == -1)
			{
				perror("write pipe");
				break;
			}
			count ++;	
		}
		
		printf("count:%d \n\n", count);
		close(pipefd[1]);
		
		exit(0);
	} 
	else if (pid > 0 )
	{
	
		sleep(4);
		close(pipefd[0]);
		close(pipefd[1]);
	}

	wait(NULL);
	printf("parent ..quit\n");
	return 0;

}

