#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <signal.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include "myipc_sem.h"
#include "myipc_shm.h"

int g_key = 0x3333;

void TestFunc(int loopnum)
{
	printf("loopnum:%d\n", loopnum);
	
	int ncount = 0;
	int shmhdl = 0;
	int *addr = NULL;
//	int ret = 0;
	
	int semid = 0;
	sem_open(g_key, &semid);


 	sem_p(semid); //临界区开始
	IPC_CreatShm(".", 0, &shmhdl);
	IPC_MapShm(shmhdl, (void **)&addr);
	*((int *)addr) =  *((int *)addr)  + 1;
	ncount = *((int *)addr);
	printf("ncount:%d\n", ncount);
	//addr[0] = addr[0] +1;
	IPC_UnMapShm(addr);
	sleep(2);
		
	sem_v(semid);  //临界区结束
	printf("进程正常退出:%d\n", getpid());
}	

int main(void )
{
    int procnum=10;
	int loopnum = 100;


	int  i=0,j = 0;

	printf("请输入要创建子进程的个数 : \n");
	scanf("%d", &procnum);

	printf("请输入让每个子进程测试多少次 :\n");
	scanf("%d", &loopnum);
	
	//共享内存创建
	int		ret = 0;
	int 	shmhdl = 0;
	ret = IPC_CreatShm(".", sizeof(int), &shmhdl);
	if (ret != 0)
	{
		printf("func IPC_CreatShm() err:%d \n", ret);
		return ret;
	}
	
	
	//信号量的创建
 	int  	semid = 0;
	ret = sem_creat(g_key, &semid);
	if (ret != 0)
	{
		printf("func sem_creat() err:%d,重新按照open打开信号量 \n", ret);
		if (ret == SEMERR_EEXIST)
		{
			ret = sem_open(g_key, &semid);
			if (ret != 0)
			{
				printf("按照打开的方式，重新获取sem失败:%d \n", ret);
				return ret;
			}
		}
		else
		{
			return ret;
		}
		
	}
	
	int  val = 0;
	ret = sem_getval(semid, &val);
	if (ret != 0 )
	{
		printf("func sem_getval() err:%d \n", ret);
		return ret;
	}
	printf("sem val:%d\n", val);
	getchar();

	pid_t pid;

	for (i=0; i<procnum; i++)
	{
		pid = fork();  //有几个fork就有几个子进程
		if (pid == 0)
		{
			for (j=0; j<loopnum; j ++)
			{
				TestFunc(j);
			}
			exit(0);
		}

	}

	//让所有的子进程退出 父进程才退出

/*
	  while(1)
	  {
		  res = wait(NULL);//
		  if(res==-1)
		  {
			  if(errno==EINTR) //若阻塞中有别的信号中断
			  {
				  continue;
			  }
			  break;
		  }
	  }
	  */
	  
	  int mypid = 0;
	  while ( (mypid= waitpid(-1, NULL, WNOHANG)) > 0)
	  {
	  	//printf("退出的子进程pid为:mypid:%d \n", mypid);
	  	;
	  }


	printf("父进程退出 hello...\n");
	return 0;
}





