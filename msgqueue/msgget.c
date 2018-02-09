#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/msg.h>


////表示打开消息队列，如果消息队列不存在，那么报错。。。。
int main01()
{
	int msgid;
	
	//表示打开文件，这个文件必须要存在
	msgid = msgget(0x1234, 0666);
	if (msgid == -1)
	{
		if (errno == ENOENT)
		{
			printf("我们自己检查，，，，消息队列不存在\n");
		}
		perror("msgget err");
		return -1;
	}
	printf("hello...\n");
	
	return 0;
}


//表示打开消息对了，若存在，就使用旧
//若不存在则创建 。
int main02()
{
	int msgid;
	
	//表示打开消息对了，若存在，就使用旧
	//若不存在则创建 
	msgid = msgget(0x1234, 0666 | IPC_CREAT);
	if (msgid == -1)
	{
		if (errno == ENOENT)
		{
			printf("我们自己检查，，，，消息队列不存在\n");
		}
		perror("msgget err");
		return -1;
	}
	printf("创建共享内存成功...\n");
	
	return 0;
}



//表示打开消息对了，若存在，就使用旧
//若不存在则创建 。
int main03()
{
	int msgid;
	
	//表示打开消息对了，若存在，就使用旧
	//若不存在则创建 
	msgid = msgget(0x1234, 0666 | IPC_CREAT);
	if (msgid == -1)
	{
		if (errno == ENOENT)
		{
			printf("我们自己检查，，，，消息队列不存在\n");
		}
		perror("msgget err");
		return -1;
	}
	printf("msgid:%d \n", msgid);
	printf("创建共享内存成功...\n");
	
	return 0;
}


//若IPC_CREAT and IPC_EXCL  在一起。。。。
//如果没有消息队列，则创建
//如果有了消息队列，则提示已经存在。。。
//若不存在则创建 。
//单独用IPC_EXCL没有意义
int main04()
{
	int msgid;
	
	//表示打开消息对了，若存在，就使用旧
	//若不存在则创建 
	msgid = msgget(0x1234, 0666 | IPC_CREAT | IPC_EXCL);
	if (msgid == -1)
	{
		if (errno == ENOENT)
		{
			printf("我们自己检查，，，，消息队列不存在\n");
		}
		if (errno == EEXIST)
		{
			printf("我们自己检查，，，，消息队已经存在\n");
		}
		
		perror("msgget err");
		return -1;
	}
	printf("msgid:%d \n", msgid);
	printf("创建共享内存成功...\n");
	
	return 0;
}

//IPC_PRIVATE 创建的消息队列，只在自己家族中使用，不在没有血缘关系的进程间用
//当我们使用了IPC_PRIVATE，   IPC_CREAT | IPC_EXCL 不起作用
int main()
{
	int msgid;
	
	//表示打开消息对了，若存在，就使用旧
	//若不存在则创建 
	//IPC_PRIVATE 表示每一次创建的消息队列不一样。。。。。所以说叫私有的
	//言外之意是说。。。。。我的msgid即使传送给其他进程，其他进程也不用。。。。（血缘关系fork,除外）
	msgid = msgget(IPC_PRIVATE, 0666);
	if (msgid == -1)
	{
		if (errno == ENOENT)
		{
			printf("我们自己检查，，，，消息队列不存在\n");
		}
		if (errno == EEXIST)
		{
			printf("我们自己检查，，，，消息队已经存在\n");
		}
		
		perror("msgget err");
		return -1;
	}
	printf("msgid:%d \n", msgid);
	printf("创建共享内存成功...\n");
	
	return 0;
}
