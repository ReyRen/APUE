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
	#include <sys/types.h>     
       #include <sys/socket.h>

       int socket(int domain, int type, int protocol);
		
	The  domain  argument  specifies  a  communication domain; this selects the protocol family which will be used for communication.
		TCP/IP只是一个家族而已

	int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
		struct sockaddr {//这是通用的结构
               		sa_family_t sa_family;
               		char        sa_data[14];
           	}
		在man 7 ip中有个指定的结构(这就是tcp/ip的结构)
		struct sockaddr_in {
               		sa_family_t    sin_family; address family: AF_INET
               		in_port_t      sin_port;   port in network byte order
               		struct in_addr sin_addr;  internet address
           	};

           Internet address
           	struct in_addr {
               		uint32_t       s_addr;     address in network byte order
           	};

	int listen(int sockfd, int backlog);
		The backlog argument defines the maximum length to which the queue of pending connections for sockfd may grow.
		this limit was a hard coded value, SOMAXCONN, with the value 128.
	
	int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
		The  argument addr is a pointer to a sockaddr structure.  This structure is filled in with the address of the peer socket, 
		The  addrlen argument is a value-result argument: the caller must initialize it to contain the size (in bytes) of the structure pointed to by addr; on r		return it will contain the actual size of the peer address.
	
		On success, these system calls return a nonnegative integer that is a descriptor for the accepted socket.  On error, -1 is returned, and errno is set  a		ppropriately.
		
		int getsockopt(int sockfd, int level, int optname,void *optval, socklen_t *optlen);
       		int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
*/
/*
为什么不能处理多个客户端，虽然显示的都已经ESTABLISHED	
	因为内核中TCP/IP协议允许不断地接收客户端的请求，所以能够握手成功establish了，但是accept去建立新的的socket的时候，服务器端一直在执行while中的接收数据，而没有
	机会去新建socket了
	解决方式：每来一个请求都得单独开进程
*/
int main(void)
{
	struct sockaddr_in srvaddr;
	//struct sockaddr peeraddr;
	struct sockaddr_in peeraddr;
	socklen_t peerlen;
	int conn;
	int ret;
	
	int sockfd = 0;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);//On success, a file descriptor for the new socket is returned.  On error, -1 is returned, and errno is set appropriately.也就是是说，socket也是个文件，返回的也是文件描述符
	if(sockfd == -1)
	{
		perror("func socket\n");
		return 1;
	}
	srvaddr.sin_family = AF_INET;
	srvaddr.sin_port = htons(8001);//将主机字节转换成网络字节
	srvaddr.sin_addr.s_addr = inet_addr("10.211.55.9");
	//srvaddr.sin_addr.s_addr = INADDR_ANY;//0.0.0.0绑定本机的任意一个地址
	/*
		SO_REUSEADDR
			服务器端尽可能使用SO_REUSEADDR
			在绑定之前尽可能调用setsockopt来设置SO_REUSEADDR socket选项。使用SO_REUSEADDR选项可以使得不必等待TIME_WAIT状态消失就可以重启服务器
				（当不用这个的时候，服务器开着，然后关了，状态会显示TIME_WAIT, 在这种状态下服务器是启动不了的，不能重新绑定端口和ip）
	*/
	int optval = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) != 0)
	{
		perror("func setsockopt\n");
		return 1;
	}

	if(bind(sockfd, (const struct sockaddr *)&srvaddr, sizeof(srvaddr)) < 0)
	{
		perror("func bind\n");
		return 1;
	}
	/*
		1.一旦调用了listen这个函数，那么这个套接字将编程被动套接字，就是只能接收链接，不能主动的发送链接
		2.listen做了两个队列，一个是已由客户端发出并达到服务器，服务器正在等待完成相应的TCP三次握手过程，另一个是完成链接的队列
			一旦三次握手队列完成，那么就放入到完成了对列中，接下来等待accept拿出来完成真正的客户端和服务器端的连接,也就是accept
			返回的那个新的socket的fd。产生了一个新的能发送链接的socket
	*/
	if(listen(sockfd, SOMAXCONN) < 0)
	{
		perror("func listen\n");
		return 1;
	}
	/*
		accept:从已完成链接队列返回第一个链接，如果已完成队列为空，则阻塞
	*/
	peerlen = sizeof(peeraddr);
	conn = accept(sockfd, (struct sockaddr *)&peeraddr, &peerlen);
	if(conn < 0)
	{
		perror("func accept\n");
		return 1;
	}
	printf("peeraddr: %s\n peerport:%d\n",inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));//char *inet_ntoa(struct in_addr in);所以不需要要再调用struct
	
	char revbuf[1024] = { 0 };
	while(1)
	{
		ret = read(conn, revbuf, sizeof(revbuf));
		if(ret == 0)
		{
			//如果在读的过程中对方已经关闭，TCP/IP协议会返回一个0数据包
			printf("对方已经已关闭\n");
			return 1;
		}
		else if(ret < 0)
		{
			printf("读数据失败\n");		
			return 1;
		}
		fputs(revbuf, stdout);//服务器端收到的数据打印屏幕
		write(conn, revbuf, ret);//服务器端回发报文，收什么发什么
	}
	return 0;
}

