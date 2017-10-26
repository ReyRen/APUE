#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(void)
{
	
	pid_t pid;


	pid = fork();
	if(pid == -1)
	{
		perror("fork err");
		return 0;
	}
	if(pid > 0)
	{
		printf("parent: %d\n", getpid());
	}
	else if(pid == 0)
	{
		printf("child :%d\n", getpid());
		execlp("ls", "ls","-lt", NULL);
	}
	return 0;
}
/*
exec函数族：
        1. execl, execlp, execle(都带l)的参数个数是可变的。参数以一个空指针结束
        2. execv和execvp的第二个参数是一个字符串数组，新程序在启动时在argv数组中给定的参数传递到main中
        3. 这些函数都是通过execve实现的，这是约定俗成
        4.名字的最后一个字母是'p'的函数会搜索PATH环境变量去查找新城虚的可执行文件，如果可执行文件不在PATH定义的路径上，就必须把包括子目录的绝对文件名做为参数
*/
