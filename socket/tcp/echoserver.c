#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

//wait(NULL) 只要有一个子进程退出，那么wait就会返回
//若多个子进程，只能等待一个。

//while (waitpid(-1, NULL, WNOHANG) > 0)
//	;
//WNOHANG 不挂起，如果没有子进程，会返回-1，从而可以跳出循环，信号函数执行完毕

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

ssize_t readn(int fd, void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nread;
	char *bufp = (char*)buf;

	while (nleft > 0)
	{
		if ((nread = read(fd, bufp, nleft)) < 0)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (nread == 0)
			return count - nleft;

		bufp += nread;
		nleft -= nread;
	}

	return count;
}

ssize_t writen(int fd, const void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nwritten;
	char *bufp = (char*)buf;

	while (nleft > 0)
	{
		if ((nwritten = write(fd, bufp, nleft)) < 0)
		{
			if (errno == EINTR)
				continue;
			return -1;
		}
		else if (nwritten == 0)
			continue;

		bufp += nwritten;
		nleft -= nwritten;
	}

	return count;
}

ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
	while (1)
	{
		int ret = recv(sockfd, buf, len, MSG_PEEK);
		if (ret == -1 && errno == EINTR)
			continue;
		return ret;
	}
}

ssize_t readline(int sockfd, void *buf, size_t maxline)
{
	int ret;
	int nread;
	char *bufp = buf;
	int nleft = maxline;
	while (1)
	{
		ret = recv_peek(sockfd, bufp, nleft);
		if (ret < 0)
			return ret;
		else if (ret == 0)
			return ret;

		nread = ret;
		int i;
		for (i=0; i<nread; i++)
		{
			if (bufp[i] == '\n')
			{
				ret = readn(sockfd, bufp, i+1);
				if (ret != i+1)
					exit(EXIT_FAILURE);

				return ret;
			}
		}

		if (nread > nleft)
			exit(EXIT_FAILURE);

		nleft -= nread;
		ret = readn(sockfd, bufp, nread);
		if (ret != nread)
			exit(EXIT_FAILURE);

		bufp += nread;
	}

	return -1;
}

void echo_srv(int conn)
{
	char recvbuf[1024];
        while (1)
        {
                memset(recvbuf, 0, sizeof(recvbuf));
                int ret = readline(conn, recvbuf, 1024); 
				if (ret == -1)
					ERR_EXIT("readline");
				if (ret == 0)
				{
					printf("client close\n");
					break;
				}
				
				if (recvbuf[0] == '2')  //注意2 一共2处。。。。
				{
					//close(conn); //11111111111
					//close(conn); //11111111111
					shutdown(conn, SHUT_WR);
					/*
						The  shutdown() call causes all or part of a full-duplex connection on the socket associated with sockfd to be shut down.  If how is SHUT_RD, further receptions
       will be disallowed.  If how is SHUT_WR, further transmissions will be disallowed.  If how is SHUT_RDWR, further receptions and transmissions will be disallowed.
					*/
				}
				
		
                fputs(recvbuf, stdout);
                writen(conn, recvbuf, strlen(recvbuf));
                
             
        }
}

void handle_sigchld(int sig)
{	
	int mypid = 0;
	
	while ( (mypid=waitpid(-1, NULL, WNOHANG)) >0 )//-1     meaning wait for any child process. 
	{
		//这这里写成循环的这种形式 是因为不可靠信号，当五个并发同时进程死了后，需要都收尸
		printf("孩子退出，父进程要收尸:%d \n", mypid);
	}
}

/*
void handle_sigchld(int sig)
{	
	int mypid = 0; 
	//wait();
	while ( (mypid=waitpid(-1, NULL, WNOHANG)) >0 )
	{
		printf("孩子退出，父进程要收尸:%d \n", mypid);
	}
}


void handle_sigchld(int sig)
{	
	int mypid = 0;
	
	if ( (mypid=waitpid(-1, NULL, WNOHANG)) >0 )
	{
		wait();
	}
}

void handle_sigchld(int sig)
{	
	//wait();
	int mypid = 0;
	
	if ( (mypid=waitpid(-1, NULL, WNOHANG)) >0 )WNOHANG     return immediately if no child has exited.
	{
		wait();
	}
}
*/


int main(void)
{
	//signal(SIGCHLD, SIG_IGN);
	
	signal(SIGCHLD, handle_sigchld);
	int listenfd;
	if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
/*	if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)*/
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8001);
	//servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	/*inet_aton("127.0.0.1", &servaddr.sin_addr);*/

	int on = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		ERR_EXIT("setsockopt");

	if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");
		
	if (listen(listenfd, SOMAXCONN) < 0)
		ERR_EXIT("listen");

	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);
	int conn;

	pid_t pid;
	while (1)
	{
		if ((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen)) < 0)
			ERR_EXIT("accept");

		printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

		pid = fork();
		if (pid == -1)
			ERR_EXIT("fork");
		if (pid == 0)
		{
			close(listenfd);
			echo_srv(conn);
			exit(EXIT_SUCCESS);
		}
		else
		{
			//close(conn);  //注意这里虽然用close了，但是没有发送FIN，因为，close的原理是引用了一个计数器，子父进程都指向了同一个文件描述符，close
					//一下，只是cout -1，只有当count=0的时候，才完全关闭，发送FIN
					//这也是文件描述符所有的规则，包括管道中dup 然后close一样

			//如果父进程这里不close，然后子进程中close一次还是发不了FIN的，就是这个原理，但是shutdown会不管你几个指向，直接down，不是引用计数
		}
			
	}

	
	return 0;
}
/*
1.父进程处理并发子进程，不能产生僵尸
	SIGCHILD处理信号
	waitpid()不要让等着
2.SIGPIPE
	忽略
3.close(conn), shutdown(conn)
4.长短链接
5.11种状态



*/
