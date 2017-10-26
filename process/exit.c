#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void bye1(void)
{
	printf("bye1....");
}
int main(void)
{
	printf("hello world\n");

	//注册终止处理程序
	atexit(bye1);	

	exit(0);
	//_exit(0); 打印不出来
	//return 0;
}
/*
_exit 系统调用,直接陷入内核，让缓冲区数据得不到刷的机会，如果要用必须前面加上fflush(stdout),注意如果上面打印有\n,\n是有清空缓冲区的作用的
exit c库函数， 会在调用的时候清除IO缓冲区，也就是临死的时候还会刷一下缓冲区，这样就能输出来hello world, 之后在执行_exit
	当exit结束的时候会主动的调终止处理程序然后清空IO,然后_exit()
*/
