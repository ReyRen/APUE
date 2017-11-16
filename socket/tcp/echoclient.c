#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

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

void echo_cli(int sock)
{
//	char sendbuf[1024] = {0};
  //      char recvbuf[1024] = {0};
     //   while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)//这里发生了阻塞，当服务器端关闭之后，服务器端端口进入半连接FIN_WAIT2状态，等待这边调用close,但是这边调用不了，所以发生一直处于半连接状态，一定要理解TCPIP的是一种状态
    //  {
      //          writen(sock, sendbuf, strlen(sendbuf));
//
  //              int ret = readline(sock, recvbuf, sizeof(recvbuf));
    //            if (ret == -1)
      //                  ERR_EXIT("readline");
        //        else if (ret == 0)
          //      {
            //            printf("client close\n");
              //          break;
                //}
//
  //              fputs(recvbuf, stdout);
    //            memset(sendbuf, 0, sizeof(sendbuf));
      //          memset(recvbuf, 0, sizeof(recvbuf));
        //}
//
  //      close(sock);

//以上的这种方式就是阻塞期间限制了两个io,一个是stdin一个是write，因为屏幕上灭有输入，所以导致write也么有进行，所以用select进行异步IO

	fd_set rset;
	FD_ZERO(&rset);

	int nready;
	int maxfd;
	int fd_stdin = fileno(stdin);
	if(fd_stdin > sock)
	{
		maxfd = fd_stdin;
	}
	else
	{
		maxfd = sock;
	}
	char sendbuf[1024] = { 0 };
	char recvbuf[1024] = {0};
	int stdineof = 0;
	while(1)
	{
		if(stdineof == 0)
		{
			FD_SET(fd_stdin, &rset);
		}
		FD_SET(sock, &rset);
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);//内核管理的文件描述符集合，检测到文件描述符发生变化了，将返回
		if(nready == -1)
		{
			ERR_EXIT("select");
		}
		if(nready == 0)//表示没有发生变化
		{
			continue;
		}
		if(FD_ISSET(sock, &rset))
		{
			int ret = readline(sock, recvbuf, sizeof(recvbuf));
			if(ret == -1)
			{
				ERR_EXIT("readline");
			}
			else if(ret == 0)
			{
				printf("server close\n");
				break;
			}
			fputs(recvbuf, stdout);
			memset(recvbuf, 0, sizeof(recvbuf));
		}
		if(FD_ISSET(fd_stdin, &rset))
		{
			if(fgets(sendbuf, sizeof(sendbuf), stdin) == NULL)
			{
				printf("ctrl + d导致fgets返回NULL");
				stdineof = 1;
				close(sock);
				/*
					close(sock);
					sleep(5);
					exit(EXIT_FAILURE);
		
					shutdown(sock, SHUT_WR);
				*/
			}
			else
			{
				//close(sock);通过测试发现如果把正常连接给关闭，会发送什么情况
				writen(sock, sendbuf, strlen(sendbuf));
				memset(sendbuf, 0, sizeof(sendbuf));
			}
		}
	}


}

void handle(int num)
{
	printf("client rec:%d\n", num);
}

/*shutdown和close的区别是：
	close是将读和写都关闭了，shutdown是可控的关闭
	一般应该服务器端调用shutdown函数，然后客户端给回个消息顺带close函数，然后服务器友好的也close了
*/
int main(void)
{
	int sock;
	signal(SIGPIPE, SIG_IGN);//当服务器端关闭后，进入FIN_WAIT1状态，然后会传个FIN值给客户端，客户端收到后进入CLOSE_WAIT状态，内核偷偷的回了服务器端w
				//然后服务器端就进入FIN_WAIT2状态，继续等待客户端调用close函数否则一直次状态。就在透透给服务器发的同时如果客户端继续向socket
				//中写数据，那么服务器会给发一个RST，然后客户端内核会给进程发一个SIGPIPE信号，直接进程消失
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8001);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
		ERR_EXIT("connect");

	struct sockaddr_in localaddr;
	socklen_t addrlen = sizeof(localaddr);
	if (getsockname(sock, (struct sockaddr*)&localaddr, &addrlen) < 0)
		ERR_EXIT("getsockname");

	printf("ip=%s port=%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));


	echo_cli(sock);

	return 0;
}

