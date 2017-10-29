#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//多进程的测试框架

void LoopFunc(int num)
{
	printf("LoopFunc()....%d\n", num);
}

int main(void)
{
	pid_t pid;
	int ret;
	int procNum = 0;
	int loopNum = 0;
	int i,j;

	printf("请输入要运行的进程数: ");
	scanf("%d", &procNum);
	printf("请输入每个进程运行圈数: ");
	scanf("%d", &loopNum);
	for(i = 0; i < procNum; i++)
	{
		pid = fork();
        	if(pid == -1) 
        	{   
                	perror("fork error");
                	return 0;
        	}  
		if(pid > 0)
        	{   
			
        	}   
        	else
        	{   
			for(j = 0; j < loopNum; j++)
			{
				LoopFunc(j);	
			}
			return 0;//让子进程跑圈，子进程不参与下一次的fork
        	}
	}
	//检测到所有子进程退出，父进程才退出
	while(1)
	{
		ret = wait(NULL);//让父进程阻塞，阻塞过程中有可能被别的信号中断...需要再做异常处理
		if(ret == -1)
		{
			if(error == EINTR)
			{
				continue;
			}
			break;
		}
	}	
	return 0;
}
/*
int main
	fork()
	fork()
	fork()
	printf("adads");//打印八次---一定要有并发运行的思维
*/
