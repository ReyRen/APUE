#include <sys/types.h>
#include <unistd.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

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


void handler(int num)
{
	printf("recv num:%d \n", num);	
	if (num == SIGQUIT)
	{
		//exit(0);
	}

}

int main(void)
{

	int n = 100;
	pid_t 	pid;
	printf("main ....begin\n");
		//SIGINT  是ctrl+c 会产生2号信号。。。 中断应用程序
	signal(SIGINT, handler);
	
	//sleep(100);
	/*  
                        也就是说，一调用sleep函数，相当于来了个system call,相当于内核让这个进程休眠了，但是休眠的过程中有可能被别的信号唤醒，一旦醒来就继续往下执行
                        这就像是之前写的multifork的wait函数，这就是为什么要判断是子进程释放让父醒来还是外界的中断让父醒来，与sleep的原理是一样的。醒来后不会再执行sleep
                        了，直接执行完下面的语句
                */
	//如果就是要向睡够
	//Zero if the requested time has elapsed, or the number of seconds left to sleep, if the call was interrupted by a signal handler.
	do 
	{
		n = sleep(n);  //sleep是可中断睡眠，让进程睡够 
		printf("要给睡够。。。。。\n");
	} while(n > 0);
	
	
	//wait();
	printf("sleep ....结束");

}
