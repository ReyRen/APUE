#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>


int main(void)
{
	pid_t pid;
	int i;

	pid = fork();
	if(pid == -1)
	{
		perror("fork error");
		exit(0);
	}
	if(pid > 0)
	{
		exit(0);
	}
	//setsid() creates a new session if the calling process is not a process group leader. 
	pid = setsid();
	if(pid == -1)
	{
		perror("setsid error");
		exit(0);
	}
	chdir("/");
	for(i = 0; i < 3; i++)
	{
		close(i);
	}
	open("/dev/null", O_RDWR);//将0号给了/dev/null
	dup(0);
	dup(1);

	while(1)
	{
		sleep(0);
	}

	return 0;
	

}
/*
守护进程：
	守护进程是在后台运行不受终端控制的进程，通常情况下守护进程在系统启动时自动运行
	守护进程的名称通常以d结尾
创建守护进程步骤
	调用fork()创建新进程，它会是将来的守护进程
	在父进程中调用exit,保证子进程不是进程组组长
	调用setsid创建新的会话期
	将当前目录改为根目录（如果当前目录作为守护进程的目录，当前目录不能被卸载，它作为守护进程的工作目录了）
	将标准输入，标准输出，标准错误重定向到/dev/null
当我们远程连接上服务器的时候，在终端和服务器之间就产生了一个session,在这个session中返回了，在这个session中有shell进程，当执行其他命令的时候有其他的进程，但是
如果想要创建守护进程，必须要用不是process froup leader的进程单独出来才能创建自己的session
*/
