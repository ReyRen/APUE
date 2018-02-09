#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

typedef struct _Teacher
{
	char name[64];
	int age;
} Teacher;


//  
int main01(int argc, char *argv[])
{
	
	int ret = 0;
	//相当于（共享内存）打开文件，共享内存文件不存在
	//
	ret = shmget(0x2234, sizeof(Teacher), 0666);
	if (ret == -1)
	{
		if (errno == ENOENT)
		{
			printf("我们自己检查共享内存不存在\n");
		}
		perror("shmget");
	}
	
	return 0;	
}

int main02(int argc, char *argv[])
{
	
	int ret = 0;
	//相当于（共享内存）打开文件，共享内存文件不存在
	//存在就使用旧的 不存在就创建
	ret = shmget(0x2234, sizeof(Teacher), 0666 | IPC_CREAT);
	if (ret == -1)
	{
		if (errno == ENOENT)
		{
			printf("我们自己检查共享内存不存在\n");
		}
		perror("shmget");
	}
	
	return 0;	
}

//如果共享内存被别的程序占用，删除共享内存，则不会立马删除。。
//出现一个现象：KYE变成了一个0.。。。。。。。。变成私有的了，也就是不能读了
//ret = shmget(0x2234, sizeof(Teacher), 0666 | IPC_CREAT |IPC_EXCL );
int main(int argc, char *argv[])
{
	
	int i=0;
	int shmid = 0;
	//如果有，报错，说共享内存已经存在
	//没有则创建。。。。。。
		//创建了共享内存
		//Teacher tArra[10];
	shmid = shmget(0x2234, 1024, 0666 | IPC_CREAT );
	if (shmid == -1)
	{
		if (errno == ENOENT)
		{
			printf("我们自己检查共享内存不存在\n");
		}
		if (errno == EEXIST)
		{
			printf("我们自己检查共享内存已经存在\n");
		}
		
		perror("shmget");
		return 0;
	}
	
	 //     void *shmat(int shmid, const void *shmaddr, int shmflg);
	void  *p = NULL; //相等于p是共享内存的首地址
	p = shmat(shmid, NULL, 0);
	Teacher t1 = {"aaadd", 34};
	
	//memcpy(p, tArra, 68)
	
	for (i=0; i<5; i++)
	{
		memcpy(p , &t1, sizeof(t1));
		memcpy(p+sizeof(t1)*i , &t1, sizeof(t1));
	}
	
	printf("键入1 表示删除 0 暂停 其他退出\n");
	int num;
	scanf("%d", &num);
	if (num == 1)
	{
		//脱离unmap
		shmdt(p);
		shmctl(shmid, IPC_RMID, NULL);
		
	}
	else if (num == 0)
	{
		pause();
	}
	else
	{
		//退出但是不删除
		//shmdt(p);
		//shmctl(shmid, IPC_RMID, NULL);
	}
	
	return 0;	
}
