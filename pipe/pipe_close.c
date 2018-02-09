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


//管道读写规则演示

#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	} while(0)


//演示管道两端的写都被关闭，读操作均返回0，不是错误
int main51(void )
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
		close(pipefd[1]);  //关闭子进程写端
		printf("child .....quit\n");
		exit(0);
	} 
	else if (pid > 0 )
	{
		int len = 0; 
		char buf[100] = {0};
		close(pipefd[1]); //关闭父进程写端
		
		//父进程sleep 1s后，再读
		sleep(1);
		
		len = read(pipefd[0], buf, 100);
		
		{
			printf("两端的写都关闭，没有报错len :%d \n", len);
		}
		
	}

	wait(NULL);
	printf("parent ..quit\n");
	return 0;

}

void myhandle(int sig)
{
	printf("recv sig:%d \n", sig);
}

//演示管道两端的读都被关闭，写操作会发生一个错误信号 SIGPIPE 管道断开的错误信号
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
		close(pipefd[0]);  //关闭子进程读端
		printf("child .....quit\n");
		exit(0);
	} 
	else if (pid > 0 )
	{
		int len = 0; 
		char buf[100] = {0};
		close(pipefd[0]); //关闭父进程读端
		
		//父进程sleep 1s后，再读
		sleep(1);
		
		len = write(pipefd[1], "wangbaomingwangbaoming", 6); 
		//会有信号发生（异步通知事件发生）言外之意主程序可以随时被打断
		//有了信号，就可以支持这种机制
		if (len == -1)
		{
			printf("两端的读都关闭，会发送信号，然后再执行这句话 :%d \n", len);
		}
	}

	wait(NULL);
	printf("parent ..quit\n");
	return 0;

}

