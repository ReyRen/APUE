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



int main(int argc, char *argv[])
{	
	//打开2.txt 准备写数据
	int outfd;
	outfd = open("./2.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644); //O_TRUNC 打开文件清空
	if (outfd == -1)
	{
		printf("open()  2.txt err..\n");
		exit(0);
	}
	
	
	//打开管道文件，准读数据
	int infd;
	infd = open("fifop", O_RDONLY);
	if (infd == -1)
	{
		printf("open err ... \n");
		exit(0);
	}
		 
	char buf[1024];
	int n = 0;
	//从文件中读数据写入管道文件
	while ( (n = read(infd, buf, 1024)) > 0)
	{
		write(outfd, buf, n);	
	}
	printf("读管道文件是否失败。。。。\n");
	close (infd);
	close (outfd);
	unlink("fifop");
	

	printf("fifor 读管道文件 success\n");
	return 0;
}

