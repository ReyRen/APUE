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
#include <sys/time.h>

/*
利用管道，两个进程间进行文件复制。
fifow
读文件1.txt	
	写入管道
fifor	
	读管道
	写入2.txt文件
	*/

int main(int argc, char *argv[])
{
	//建立管道文件
	mkfifo("fifop", 0644);
	int infd;
	
	//按只读方式 打开现有文件
	infd = open("./1.txt", O_RDONLY);
	if (infd == -1)
	{
		printf("infd open() err..\n");
		exit(0);
	}
	int outfd;
	
	//打开管道文件，准备写数据
	//outfd = open("fifop", O_WRONLY | O_NONBLOCK);
	outfd = open("fifop", O_WRONLY ); //阻塞模式
	if (outfd == -1)
	{
		printf("infd open() err..\n");
		exit(0);
	}
	
	char buf[1024];
	int n = 0;
	//从文件中读数据写入管道文件
	while ( (n = read(infd, buf, 1024)) > 0)
	{
		write(outfd, buf, n);	
	}
	close (infd);
	close (outfd);
	

	printf("fifow 写管道文件 success\n");
	return 0;
}

