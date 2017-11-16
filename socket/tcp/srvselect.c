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
		
                fputs(recvbuf, stdout);
                writen(conn, recvbuf, strlen(recvbuf));
        }
}

void handle_sigchld(int sig)
{
/*	wait(NULL);*/
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
}

void handle_sigpipe(int sig)
{
	printf("recv a sig=%d\n", sig);
}

int main(void)
{
	/*signal(SIGPIPE, SIG_IGN);*/
	signal(SIGPIPE, handle_sigpipe);
/*	signal(SIGCHLD, SIG_IGN);*/
	signal(SIGCHLD, handle_sigchld);
	int listenfd;
	if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
/*	if ((listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)*/
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	/*servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");*/
	/*inet_aton("127.0.0.1", &servaddr.sin_addr);*/

	int on = 1;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
		ERR_EXIT("setsockopt");

	if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("bind");
	if (listen(listenfd, SOMAXCONN) < 0)
		ERR_EXIT("listen");

	struct sockaddr_in peeraddr;
	socklen_t peerlen;	
	int conn;

/*
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
			close(conn);
	}
*/

	int i;
	int client[FD_SETSIZE]; //支持的最大客户端连接数 数组
	int maxi = 0; //最大的不空闲的位置

	printf("FD_SETSIZE:%d \n", FD_SETSIZE);
	for (i=0; i<FD_SETSIZE; i++)
		client[i] = -1;

	int nready;
	int maxfd = listenfd;
	fd_set 		rset;
	fd_set 		allset;
	
	FD_ZERO(&rset);
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	printf("FD_SETSIZE:%d \n", FD_SETSIZE);
	while (1)
	{
		rset = allset;
		//listenfd 可读，表示有客户端连接进来了，可以建立一个新的连接
		nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		if (nready == -1)
		{
			if (errno == EINTR)
				continue;
			
			ERR_EXIT("select");
		}
		if (nready == 0)
			continue;

		if (FD_ISSET(listenfd, &rset)) //若侦听套接口产生可读事件 说明3次握手已完成，有客户端已经连接建立
		{
			peerlen = sizeof(peeraddr);
			conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen); //处理accept
			if (conn == -1)
				ERR_EXIT("accept");

			for (i=0; i<FD_SETSIZE; i++)
			{
				if (client[i] < 0)
				{
					client[i] = conn;
					if (i > maxi)
						maxi = i;
					break;
				}
			}

			if (i == FD_SETSIZE)
			{
				fprintf(stderr, "too many clients\n");
				exit(EXIT_FAILURE);
			}
			printf("ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));
			
			FD_SET(conn, &allset); //把新的连接 放入集合中
			if (conn > maxfd)  //同时按照条件，更新maxfd  //维护最大描述符
				maxfd = conn;

			if (--nready <= 0)
				continue;

		}

		for (i=0; i<=maxi; i++) //检测已连接的调节字conn是否可读
		{
			conn = client[i];
			if (conn == -1)
				continue;

			if (FD_ISSET(conn, &rset)) //当前连接是否可读
			{
				char recvbuf[1024] = {0};
				int ret = readline(conn, recvbuf, 1024);
        		if (ret == -1)
                		ERR_EXIT("readline");
        		if (ret == 0)
        		{
            		printf("client close\n");
					FD_CLR(conn, &allset); //若对方已退出 从集合中清除
					client[i] = -1; //保存连接的数组 也置成-1 //也可进一步控制 若i是把maxfd，需要把maxifd变成第二大maxifd
					close(conn);
        		}

        		fputs(recvbuf, stdout);
				//sleep(4);
        		writen(conn, recvbuf, strlen(recvbuf));

				if (--nready <= 0)
					break;	
			}
		}
	}	
	return 0;
}

