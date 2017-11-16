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
#include <arpa/inet.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/*

       uint32_t htonl(uint32_t hostlong);

       uint16_t htons(uint16_t hostshort);

       uint32_t ntohl(uint32_t netlong);

       uint16_t ntohs(uint16_t netshort);
*/

int main11(void)
{
	unsigned int data = 0x12345678;//最右面是低位
	char *p = (char *)&data;
	printf("%x, %x %x %x \n", p[0], p[1], p[2], p[3]);//p[0]表示低内存
	
	if ( p[0] == 0x78)
	{
			printf("当前系统是小端模式\n");
	}
	else
	{
		//低位放在高内存处就是大端模式
		printf("当前系统是大端模式\n");
	}
	//x86平台是小端模型   ibm主机 8大端 
	
	printf("把本地字节转换成网络字节\n");
	 uint32_t mynetdat = htonl(data);
	 p = (char *)&mynetdat;
	 printf("%x, %x %x %x \n", p[0], p[1], p[2], p[3]);
	if ( p[0] == 0x78)
	{
			printf("网络字节序是是小端模式\n");
	}
	else
	{
		printf("网络字节序是是大端模式\n");
	}
	return 0;
}

int main()
{
	/*
	  int inet_aton(const char *cp, struct in_addr *inp); //1

       in_addr_t inet_addr(const char *cp); //2

       in_addr_t inet_network(const char *cp);

       char *inet_ntoa(struct in_addr in); //4

       struct in_addr inet_makeaddr(int net, int host);

       in_addr_t inet_lnaof(struct in_addr in);

       in_addr_t inet_netof(struct in_addr in);
       */
       
       //struct in_addr *inp;
       
         int  myint = inet_addr("192.168.6.222"); //2
         printf("%u\n", myint);
         
         struct in_addr myaddr;
         
       
         inet_aton("192.168.6.222", &myaddr); //1
         printf("%u\n", myaddr.s_addr);
         
         
         
          //char *inet_ntoa(struct in_addr in); //4
          //请你深刻的理解为什么这个地方 要求传入的是元素
          printf("%s\n",inet_ntoa(myaddr));
         /*
             //Internet address. 
         struct in_addr {
             u_int32_t      s_addr;     // address in network byte order 
         };
         */
	return 0;
}
