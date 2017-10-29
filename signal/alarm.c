#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void myhandle(int num)
{
	printf("recv signal id num : %d \n", num);
	//alarm(1);这样就很容易的实现了每一秒中发送一个alarm信号
}

int main(void)
{
	printf("main....begin\n");
	if(signal(SIGALRM. myhandle) == SIG_ERR)
	{
		perror("func signal err\n");
		return 0;
	}
	alarm(1);//和sleep和pause，wait一样，执行完就不会执行这句了，会接着往下走，除非循环起来
	while(1)
	{
		pause();
		printf("pause return\n");
	}
	return 0;
}

