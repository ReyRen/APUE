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


int main01(void )
{
	int pipefd[2];
	pid_t pid;
	
	//创建管道文件，并打开
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
		len = read(pipefd[0], buf, 100);
		printf("len:%d, buf:%s \n", len , buf);

		close(pipefd[0]);
	}

	wait(NULL);
	printf("parent ..quit\n");
	return 0;

}

//阻塞非阻塞场景
int main1111(void )
{
	int ret = 0;
	int	pipefd[2];
	
	ret = pipe(pipefd);
	if (ret < 0)
	{
		perror("ddd");
		exit(0);
	}
	int pid; 
	pid = fork();
	if (pid == -1)
	{
		perror("dd");
		exit(0);
	}
	
	if (pid == 0) //子进程
	{
		close(pipefd[0]);
		sleep(5);
		write(pipefd[1], "childaaaaaaaaa", 6 );
		printf("chile quit\n");
		close(pipefd[1]);
		exit(0);
	}
	
	close(pipefd[1]);
	char readbuf[1000] = {0};
	int n = 0;
	
	int flags = fcntl(pipefd[0], F_GETFL);
		flags = flags | O_NONBLOCK;
		 ret = fcntl(pipefd[0], F_SETFL, flags);
		if (ret == -1)
		{
			printf("fcntl err.\n");
			exit(0);
		}
		
	n = read(pipefd[0], readbuf,sizeof(readbuf));
	if (n < 0)
	{
		printf("read非阻塞返回。。。。。立马返回");
		//errno
	}
	printf("%s\n", readbuf);
	close(pipefd[0]);
	
	wait(NULL);
	
	printf("parent quit\n");
}

//阻塞非阻塞场景
int main(void )
{
	int ret = 0;
	int	pipefd[2];
	
	ret = pipe(pipefd);
	if (ret < 0)
	{
		perror("ddd");
		exit(0);
	}
	int pid; 
	pid = fork();
	if (pid == -1)
	{
		perror("dd");
		exit(0);
	}
	
	if (pid == 0) //子进程
	{
		close(pipefd[0]);
		//sleep(5);
		//write(pipefd[1], "childaaaaaaaaa", 6 );
		//printf("chile quit\n");
		close(pipefd[1]);
		exit(0);
	}
	
	close(pipefd[1]);
	char readbuf[1000] = {0};
	int n = 0;
	sleep(3);
	
	n = read(pipefd[0], readbuf,sizeof(readbuf));
	printf("所有的写端都关闭 。。。。 n:%d \n", n);
	
	printf("%s\n", readbuf);
	close(pipefd[0]);
	
	wait(NULL);
	
	printf("parent quit\n");
}

