#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "commonsocketT.h"

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)
        
        

void handle(int signum)
{
	int pid = 0;
	printf("recv signum:%d \n", signum);
	
	//avoid zombie
	while ((pid = waitpid(-1, NULL, WNOHANG) ) > 0)
	{
		printf("exit child process pid %d \n", pid);
		fflush(stdout);
	} 
}

int main(void)
{
	int	ret = 0;//result set
	int 	listenfd;//listen socket fd
	
	
	signal(SIGCHLD, handle);//handle the child died
	signal(SIGPIPE, SIG_IGN);//handle the pipe broken
	

	ret = sckServer_init(8001, &listenfd);
	if (ret != 0)
	{
		printf("sckServer_init() err:%d \n", ret);
		return ret;
	}
	
	while(1)
	{
		int ret = 0;
		int wait_seconds = 20;
		int connfd = 0;

		ret = sckServer_accept(listenfd, &connfd,  wait_seconds);
		if (ret == Sck_ErrTimeOut)
		{
			printf("timeout....\n");
			continue;
		}
	
		int pid = fork();
		if (pid == 0)
		{
			unsigned char 	recvbuf[1024];
			int 	recvbuflen = 1024;
				
			close(listenfd);
			while(1)
			{
				memset(recvbuf, 0, sizeof(recvbuf));
				ret = sckServer_rev(connfd, recvbuf, &recvbuflen,  20);
				if (ret != 0)
				{
					printf("func sckServer_rev() err:%d \n", ret);
					break;
				}
				printf("%s\n", recvbuf);
				
				
				ret = sckServer_send(connfd,  recvbuf, recvbuflen, 20);
				if (ret != 0)
				{
					printf("func sckServer_send() err:%d \n", ret);
					break;
				}
			}
			close(connfd);
			exit(ret);
		}
		else if (pid > 0)
		{
			close(connfd);
		}
	}
	

	return 0;
}

