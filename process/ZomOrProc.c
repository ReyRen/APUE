#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(void)
{
	printf("hello....\n");
	
	pid_t pid;
	printf("before fork pid:%d\n", getpid());
	//信号注册函数
	signal(SIGCHLD, SIG_IGN);//SIG_IGN表示忽略信号，也就用SIGCHLD信号告诉内核，当孩子死后，我就不管了

	pid = fork();
	if(pid == -1)
	{
		perror("fork err");
		return 0;
	}
	if(pid > 0)
	{
		printf("parent: %d\n", getpid());
		sleep(10);//僵尸
	}
	else if(pid == 0)
	{
		printf("child :%d\n", getpid());
		//sleep(10);//这样父亲先死了
	}

	printf("after fork \n");
	return 0;
}
/*
进程的创建：
	在Linux内核中创建
0号进程和1号进程
	0号进程，linux内核的服务进程。在linux完成自举，自己给自己创建一个进城
	0号进程也叫空闲进程
	0号进程会产生第一个用户空间进程。。。。1号进程
孤儿进程
	parent先死，子进程托给一号进程
僵尸进程
	孩子先死，Parent没来得及收尸
避免僵尸进程的方法是
	创建子进程的时候可以不管子进程，让Linux内核去管（发送信号给kernal）
		信号就是异步处理事件，是一种机制。也就是说我的程序顺序执行的同时，能支持异步的信号调用处理函数
*/
