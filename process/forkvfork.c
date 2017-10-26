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
	int num;

	pid = vfork();
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
		//char *const argv[] = {"ls", "-lt", NULL};
		//hello的代码段数据段，内存控制块儿还有堆栈段完全覆盖vfork子进程，只有vfork子进程的pid不被覆盖
		execve("./hello", NULL, NULL);
		printf("child :%d\n", getpid());
	}

	return 0;
}
/*
fork与vfork:
	1.fork子进程拷贝父进程的数据段
	2.vfork子进程与父进程共享数据段
	3.fork父子进程执行的次序不一定
	4.vfork子进程先运行，父进程后运行
vfork必须要和exec族在一起，不然必须exit(0/1)，如果使用return啥的就存在down的不稳定情况

*/
