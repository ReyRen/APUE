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


//测试 默认情况下，管道的读写
int main41(void )
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
		printf("子进程延迟3秒后，再写。。\n");
		sleep(3);
		close(pipefd[0]);
		write(pipefd[1], "hello hello....", 6);
		close(pipefd[1]);
		printf("child .....quit\n");
		exit(0);
	} 
	else if (pid > 0 )
	{
		int len = 0; 
		char buf[100] = {0};
		close(pipefd[1]);
	
	
		printf("测试非阻塞情况下，begin read ...\n");
		len = -1;
		while (len < 0)
		{
				len = read(pipefd[0], buf, 100);
				if (len == -1)
				{
					sleep(1);
					//close(pipefd[0]);
					perror("\nread err:");
					//exit(0);
				}
		}
	
		printf("len:%d, buf:%s \n", len , buf);
		close(pipefd[0]);
	}

	wait(NULL);
	printf("parent ..quit\n");
	return 0;

}


//文件状态设置成阻塞非阻塞
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
	
	/*	
       int fcntl(int fd, int cmd);
       int fcntl(int fd, int cmd, long arg);
       int fcntl(int fd, int cmd, struct flock *lock);
F_GETFL  F_SETFL O_NONBLOCK
//注意不要设置错误成F_GETFD  F_SETFD


*/
		int flags = fcntl(pipefd[0], F_GETFL);
		flags = flags | O_NONBLOCK;
		int ret = fcntl(pipefd[0], F_SETFL, flags);
		if (ret == -1)
		{
			printf("fcntl err.\n");
			exit(0);
		}
		
		//把pipefd[0]文件描述符修改成非阻塞 man fcntl
		printf("begin read ...\n");
		len = -1;
		while (len < 0)
		{
				len = read(pipefd[0], buf, 100);
				if (len == -1)
				{
					sleep(1);
					//close(pipefd[0]);
					perror("read err.\n");
					//exit(0);
				}
		}
	
		printf("len:%d, buf:%s \n", len , buf);
		close(pipefd[0]);
	}

	wait(NULL);
	printf("parent ..quit\n");
	return 0;

}

