#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "commonsocketT.h"
/*
void handle(int signum)
{
	int pid = 0;
	printf("recv signum:%d \n", signum);
	
	//avoid the zombie
	while ((pid = waitpid(-1, NULL, WNOHANG) ) > 0)
	{
		printf("exit child process pid%d \n", pid);
		fflush(stdout);
	} 
}
*/
int main(void)
{	
	int 		ret = 0;//result set
	void 		*handle = NULL;//store struct 
	int 		connfd;//connect socket fd
	unsigned char *data = (unsigned char *)"aaaaaafffffffffffssssss";//data
	int 		datalen = 10;//send data length
	unsigned char out[1024];//receive data storage
	int outlen = 1024;//receive length

	//int sckCliet_init(void **handle, char *ip, int port, int contime, int sendtime, int revtime);
	ret = sckCliet_init(&handle, 15, 5, 5, 10);
	if(ret != 0)
	{
		printf("sckCliet_init err: %d\n", ret);
		return ret;
	}
	
	ret = sckCliet_getconn(handle, "127.0.0.1", 8001, &connfd);
 	if(ret != 0)
        {   
                printf("sckCliet_getconn err: %d\n", ret);
		return ret;
        }
	//client send
	ret = sckClient_send(handle,  connfd, data, datalen);
	if(ret != 0)
        {   
                printf("sckClient_send err: %d\n", ret);
		return ret;
        }
//	if (ret == Sck_ErrTimeOut)
//	{
		//ret = sckClient_send(handle,  connfd, data, datalen);
//	}
//	printf("ccccccc\n");
	
	//client receive
	ret = sckClient_rev(handle, connfd, out, &outlen);
	if(ret != 0)
        {   
                printf("sckClient_rev err: %d\n", ret);
		return ret;
        }
	out[outlen] = '\0';
	printf("data: %s \n", out);
	
	//release 
	ret = sckClient_destroy(handle);
	if(ret != 0)
        {
                printf("sckClient_destroy err: %d\n", ret);
		return ret;
        }
	return ret;
}


