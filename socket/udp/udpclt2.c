#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

void echo_cli(int sock)
{
	struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8002);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//3 udp 也可以 调用connet
	//udp调用connet，并没有三次握手，只是维护了一个状态信息（和对等方的）。。。
	connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr));

	int ret;
	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};
	while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{	
		//2如果	connect 已经指定了对方的地址。
		//send可以这样写 sendto(sock, sendbuf, strlen(sendbuf), 0, NULL, 0);
		
		//1sendto第一次发送的时候，会绑定地址
		sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
		/*sendto(sock, sendbuf, strlen(sendbuf), 0, NULL, 0);*/
		
		//一但调用connect，就可以使用send函数
		//send(sock, sendbuf, strlen(sendbuf), 0);
		ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
		if (ret == -1)	
		{
			if (errno == EINTR) 
				continue;
			ERR_EXIT("recvfrom");
		}

		fputs(recvbuf, stdout);
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}

	close(sock);


}

int main(void)
{
	int sock;
	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
		ERR_EXIT("socket");

	echo_cli(sock);

	return 0;
}

/*
	客户端调用connect和不调用connect的区别:
		udp也可以调用connect
		客户端调用了connect以后不会阻塞在recvfrom函数里，会返回ICMP报错
*/
