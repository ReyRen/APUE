#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//父子进程共享文件描述符情况
//相当于两个fd指向同一块儿内存空间而已
//因为两个进程共享了文件指针偏移量，所以都能向文件中写数据
int main(void)
{
	
	pid_t pid;
	int fd;

	fd = open("test.txt", O_WRONLY);
	if(fd == -1)
	{
		perror("open err");
		return 1;
	}

	pid = fork();
	if(pid == -1)
	{
		perror("fork err");
		return 0;
	}
	if(pid > 0)
	{
		write(fd, "parent", 6);
		printf("parent: %d\n", getpid());
		close(fd);
	}
	else if(pid == 0)
	{
		write(fd, "child", 5);
		printf("child :%d\n", getpid());
		close(fd);
	}
	sleep(2);

	return 0;
}
