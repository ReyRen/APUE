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

//wait函数就相当于是让父进程阻塞，等待获取子进程的状态才会醒来
int main(int argc, char *argv[])
{
	pid_t pid;
	pid = fork();
	if (pid == -1)
		ERR_EXIT("fork error");

	if (pid == 0)
	{
		sleep(3);
		printf("this is child\n");
		//exit(100);
		abort(); //非正常退出 //告诉linux内核产生一个SIGABRT 6号信号
	}

	int ret;
	printf("this is parent\n");
	int status;
	//ret = wait(&status);== waitpid(-1, &status, 0);
	/*
	pid_t waitpid(pid_t pid, int *status, int options);
		 The value of pid can be:

       < -1   meaning wait for any child process whose process group ID is equal to the absolute value of pid.

       -1     meaning wait for any child process.

       0      meaning wait for any child process whose process group ID is equal to that of the calling process.

       > 0    meaning wait for the child whose process ID is equal to the value of pid.
	*/
	ret = waitpid(-101, &status, 0);
	//ret = waitpid(pid, &status, 0);
	printf("ret = %d pid = %d\n", ret, pid);
	if ( WIFEXITED(status) ) //wait if exit ed  
	{
		 printf("进程退出: %d \n", WEXITSTATUS(status));//就是返回了exit(100)中的100这个正常退出的状态码
	}
	else if (WIFSIGNALED(status))
	{
		printf("进程非正常退出: %d \n", WTERMSIG(status) );
	}
	else if  ( WIFSTOPPED(status))
	{
		printf("进程停止: %d \n", WSTOPSIG(status) );
	}
	else
	{
		printf("其他方式退出 \n");
	}
	

	
	/*
       WIFEXITED(status)
              returns  true  if the child terminated normally, that is, by calling exit(3) or _exit(2), or by returning
              from main().

       WEXITSTATUS(status)
              returns the exit status of the child.  This consists of the least significant 16-8  bits  of  the  status
              argument  that  the child specified in a call to exit() or _exit() or as the argument for a return state-
              ment in main().  This macro should only be employed if WIFEXITED returned true.

*/
	
	
}
