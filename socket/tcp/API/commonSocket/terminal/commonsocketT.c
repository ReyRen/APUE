#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "commonsocketT.h"

typedef struct _SckHandle
{
	int sockArray[100]; //socket pool
	int arrayNum; //how man connect to the server
	int sockfd; //socket fd
	int contime; //the max time of conn
	int sendtime; //the max time of send
	int revtime; //the max time of rcv
} SckHandle;

/**
 * readn - read the specified bytes
 * @fd: file descriptor
 * @buf: buffer
 * @count: count of the bytes
 * success: return count
   fail:    return -1
   EPF:     return < count
 */
/*
 * func name：readn
 * description:：client rcv msg
 * */
ssize_t readn(int fd, void *buf, size_t count)
{
	//size_t x64 is unsigned long， x86 is unsigned int
	size_t nleft = count;
	ssize_t nread;
	char *bufp = (char*) buf;
	while (nleft > 0)
	{
		/*ssize_t read(int fd, void *buf, size_t count);
		 * read count bytes from fd to buf
		 * return the amount of read
		 * */
		if ((nread = read(fd, bufp, nleft)) < 0)
		{
			/*
			 * if interruped by the signal
			 * */
			if (errno == EINTR)
			{
				continue;
			}
			return -1;
		}
		/*
		 * if the count = 0, it means the nread would be also 0.
		 * do not do any operation
		 * nleft is the left bytes need to be read
		 * if nread = 0 it means the end of the file
		 *
		 * */
		else if (nread == 0)
		{
			return count - nleft;//it mnens the actually read bytes
		}
		bufp += nread; //forward the pointer to the left bytes. 
		nleft -= nread; 
	}
	return count;
}

/**
 * writen - send the specific amount of bytes
 * @fd: file descriptor
 * @buf: send buf
 * @count: the amount bytes need to be wrote
 * success:count
   fail: -1
 */
ssize_t writen(int fd, const void *buf, size_t count)
{
	size_t nleft = count;
	ssize_t nwritten; //success bytes of wrote
	char *bufp = (char*) buf;
	/*
	 * if the left > 0, it would be continue
	 * */
	while (nleft > 0)
	{
		/*
		 * ssize_t write(int fd, const void *buf, size_t count);
		 *
		 * */
		if ((nwritten = write(fd, bufp, nleft)) < 0)
		{
			/*
			 * if interruped by signal, continue
			 * */
			if (errno == EINTR)
			{
				continue;
			}
			return -1;
		} 
		else if (nwritten == 0)
		{
			continue;
		}
		//pointer forward
		bufp += nwritten;
		//left bytes
		nleft -= nwritten;
	}
	return count;
}

/**
 * recv_peek - only check data in the buffer, not flush
 * @sockfd: fd
 * @buf: rcv buf
 * @len: length
 * success: return >= 0
   fail: -1
 */
ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
	while (1)
	{
		/*
		 * ssize_t recv(int sockfd, void *buf, size_t len, int flags);
		 * sockfd
		 * len read length
		 * MSG_PEEK check, don't flush
		 * success: read bytes
		   fail: -1
       		   close: 0
		 * */
		int ret = recv(sockfd, buf, len, MSG_PEEK);
		/*
		 * interruped by signal
		 * */
		if (ret == -1 && errno == EINTR)
		{
			continue;
		}
		return ret;
	}
}

//client init
/*
 * handle socket struct
 * contime connect timeout 
 * sendtime send timeout
 * revtime rcv timeout
 * nConNum socketfd num
 * */
int sckCliet_init(void **handle, int contime, int sendtime, int revtime,
		int nConNum)
{
	int ret = 0;

	if (handle == NULL || contime < 0 || sendtime < 0 || revtime < 0)
	{
		ret = Sck_ErrParam;
		printf("func sckCliet_init() err: %d, check  (handle == NULL ||contime<0 || sendtime<0 || revtime<0)\n", ret);
		return ret;
	}
	//define the struct
	SckHandle *tmp = (SckHandle *) malloc(sizeof(SckHandle));
	if (tmp == NULL)
	{
		ret = Sck_ErrMalloc;
		printf("func sckCliet_init() err: malloc %d\n", ret);
		return ret;
	}

	tmp->contime = contime;
	tmp->sendtime = sendtime;
	tmp->revtime = revtime;
	tmp->arrayNum = nConNum;
	*handle = tmp;
	return ret;
}

/**
 * activate_noblock - set the I/O is the non-block mode
 * @fd: file descriptor
 */
int activate_nonblock(int fd)
{
	int ret = 0;
	/*
	 * int fcntl(int fd, int cmd, ... \* arg \*)
	 got the file status or modified the file status
	 F_GETLK: Get the file access mode and the file status flags
	 For a successful call F_GETFL  Value of file status flags.
	 */
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1)
	{
		ret = flags;
		printf("func activate_nonblock() err:%d", ret);
		return ret;
	}
	/*
	 *  add the non-block status
	 * */
	flags |= O_NONBLOCK;
	/*
	 * F_SETFL ：Set the file descriptor flags to the value specified by arg
	 O_RDONLY， O_WRONLY， O_RDWR， O_CREAT， O_EXCL， O_NOCTTY 和 O_TRUNC would not be affected. 
	 which one can modified: O_APPEND，O_ASYNC， O_DIRECT， O_NOATIME 和 O_NONBLOCK。
	 * */
	ret = fcntl(fd, F_SETFL, flags);
	if (ret == -1)
	{
		printf("func activate_nonblock() err:%d", ret);
		return ret;
	}
	return ret;
}

/**
 * deactivate_nonblock - set the I/O blocked
 * @fd: file descriptor
 */

int deactivate_nonblock(int fd)
{
	int ret = 0;
	/*
	 * int fcntl(int fd, int cmd, ... \* arg \*)
	 * same with the activate_nonblock
	 */
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1)
	{
		ret = flags;
		printf("func deactivate_nonblock() err:%d", ret);
		return ret;
	}

	/*
	 * bit &， NONBLOCK ~
	 * */
	flags &= ~O_NONBLOCK;
	/*
         * same with the activate_nonblock
	 * */
	ret = fcntl(fd, F_SETFL, flags);
	if (ret == -1)
	{
		printf("func deactivate_nonblock() err:%d", ret);
		return ret;
	}
	return ret;
}

/**
 * connect_timeout - connect
 * @fd: socket fd
 * @addr: the address of server
 * @wait_seconds: timeout time. If this parameter is 0, it means no wait time out there
 * Success: 0
   fail: -1
   timeout: -1 and errno = ETIMEDOUT
 */
/*
 *
 *struct sockaddr_in {
 *	 sa_family_t    sin_family; // address family: AF_INET
 *	 in_port_t      sin_port;   // port in network byte order
 *	 struct in_addr sin_addr;   // internet address
 *};
 * */
// static means used just in this file. Outer users cannot use.

static int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
	int ret;
	//got the struct size
	socklen_t addrlen = sizeof(struct sockaddr_in);
	//if the wait_seconds > 0 canceled the socket block, if 0 go the default( block )
	if (wait_seconds > 0)
		activate_nonblock(fd);
	/*
	 * int connect(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
	 *
	 * */
	ret = connect(fd, (struct sockaddr*) addr, addrlen);//if the netwrok is perfect got the >0 ret is perfect
	//EINPROGRESS means handling
	if (ret < 0 && errno == EINPROGRESS)
	{
		/*
		 * void FD_CLR(int fd, fd_set *set);
		 * int  FD_ISSET(int fd, fd_set *set);
		 * void FD_SET(int fd, fd_set *set);
		 * void FD_ZERO(fd_set *set);
		 * */
		//set the select sets. 
		fd_set connect_fdset;
		struct timeval timeout;
		//init the sets
		FD_ZERO(&connect_fdset);
		//put the socket fd into the sets.
		FD_SET(fd, &connect_fdset);
		/*
		 * struct timeval {
		 *     long    tv_sec;         // seconds 
		 *     long    tv_usec;        // microseconds 
		 *     };
		 * */
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			// onec connect establish, the fd can write
			ret = select(fd + 1, NULL, &connect_fdset, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);
		if (ret == 0)
		{
			ret = -1;
			/*
			 * #define ETIMEDOUT       110     // Connection timed out
             		 *  When the tcp detected that the peer socket no longer useful, 
             		 *  select would return socket readble, and retrun -1 errno is set ETIMEDOUT
			 * */
			errno = ETIMEDOUT;
		} else if (ret < 0)
			return -1;
		else if (ret == 1)
		{
			/* ret is 1（this socket is writeable），there are two reasons: connect success or socket got wrong*/
			/* the err msg would not be stored into the errno. So. it needs to use getsockopt to get */
			int err;
			socklen_t socklen = sizeof(err);
			//got the socket status
			int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
			if (sockoptret == -1)
			{
				return -1;
			}
			if (err == 0)
			{
				ret = 0;
			} else
			{
				errno = err;
				ret = -1;
			}
		}
	}
	if (wait_seconds > 0)
	{
		deactivate_nonblock(fd);
	}
	return ret;
}

//connect
int sckCliet_getconn(void *handle, char *ip, int port, int *connfd)
{

	int ret = 0;
	SckHandle *tmp = NULL;
	if (handle == NULL || ip == NULL || connfd == NULL || port < 0 || port > 65537)
	{
		ret = Sck_ErrParam;
		printf( "func sckCliet_getconn() err: %d, check  (handle == NULL || ip==NULL || connfd==NULL || port<0 || port>65537) \n", ret);
		return ret;
	}

	int sockfd;
	sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0)
	{
		ret = errno;
		printf("func socket() err:  %d\n", ret);
		return ret;
	}

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip);

	tmp = (SckHandle*) handle;

	/*
	 ret = connect(sockfd, (struct sockaddr*) (&servaddr), sizeof(servaddr));
	 if (ret < 0)
	 {
	 ret = errno;
	 printf("func connect() err:  %d\n", ret);
	 return ret;
	 }
	 */

	ret = connect_timeout(sockfd, (struct sockaddr_in*) (&servaddr), (unsigned int) tmp->contime);
	if (ret < 0)
	{
		if (ret == -1 && errno == ETIMEDOUT)
		{
			ret = Sck_ErrTimeOut;
			return ret;
		} else
		{
			printf("func connect_timeout() err:  %d\n", ret);
		}
	}

	*connfd = sockfd;

	return ret;

}

/**
 * write_timeout - write timeout
 * @fd: file descriptor
 * @wait_seconds: timeout time, 0 is no wait time
 * success: 0
   fail: -1 and errno = ETIMEDOUT
 */
int write_timeout(int fd, unsigned int wait_seconds)
{
	int ret = 0;
	if (wait_seconds > 0)
	{
		fd_set write_fdset;
		struct timeval timeout;

		FD_ZERO(&write_fdset);
		FD_SET(fd, &write_fdset);

		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			ret = select(fd + 1, NULL, &write_fdset, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);

		if (ret == 0)
		{
			ret = -1;
			errno = ETIMEDOUT;
		} else if (ret == 1)
			ret = 0;
	}

	return ret;
}

//client send msg
/*
 * func name：sckClient_send
 * description：client send msg
 * */
int sckClient_send(void *handle, int connfd, unsigned char *data, int datalen)
{
	int ret = 0;

	SckHandle *tmp = NULL;
	tmp = (SckHandle *) handle;
	ret = write_timeout(connfd, tmp->sendtime);
	if (ret == 0)
	{
		int writed = 0;
		unsigned char *netdata = (unsigned char *) malloc(datalen + 4);
		if (netdata == NULL)
		{
			ret = Sck_ErrMalloc;
			printf("func sckClient_send() mlloc Err:%d\n ", ret);
			return ret;
		}
		//purpose: prevent the stick package situation
		int netlen = htonl(datalen);
		memcpy(netdata, &netlen, 4);//the first 4 bytes is the length of specific package 
		memcpy(netdata + 4, data, datalen);

		writed = writen(connfd, netdata, datalen + 4);
		if (writed < (datalen + 4))
		{
			if (netdata != NULL)
			{
				free(netdata);
				netdata = NULL;
			}
			return writed;
		}

	}

	if (ret < 0)
	{
		//fail return -1 and errno=ETIMEDOUT
		if (ret == -1 && errno == ETIMEDOUT)
		{
			ret = Sck_ErrTimeOut;
			printf("func sckClient_send() mlloc Err:%d\n ", ret);
			return ret;
		}
		return ret;
	}

	return ret;
}

/**
 * read_timeout - read timeout
 * @fd: file descriptor
 * @wait_seconds: timeout time, 0 is not wait
 * success: 0
   fail: -1 and errno = ETIMEDOUT
 */
//client rcv msg
int read_timeout(int fd, unsigned int wait_seconds)
{
	int ret = 0;
	if (wait_seconds > 0)
	{
		fd_set read_fdset;
		struct timeval timeout;

		FD_ZERO(&read_fdset);
		FD_SET(fd, &read_fdset);

		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;

		//select return values:
		// if timeout, and no detected the read happened, retrun 0
		// if ret < 0 && errno == EINTR, interruped by other signal
		// select fail return -1
		// if the ret > 0, it means there are read happened, so return the amount of happenend. 

		do
		{
			ret = select(fd + 1, &read_fdset, NULL, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);

		if (ret == 0)
		{
			ret = -1;
			errno = ETIMEDOUT;
		} else if (ret == 1)
			ret = 0;
	}

	return ret;
}

//client rcv msg
/*
 * func name：sckClient_rev
 * description：client rcv msg
 * */
int sckClient_rev(void *handle, int connfd, unsigned char *out, int *outlen)
{

	int ret = 0;
	SckHandle *tmpHandle = (SckHandle *) handle;

	if (handle == NULL || out == NULL)
	{
		ret = Sck_ErrParam;
		printf("func sckClient_rev() timeout , err:%d \n", Sck_ErrTimeOut);
		return ret;
	}

	ret = read_timeout(connfd, tmpHandle->revtime); //bugs modify bombing
	if (ret != 0)
	{
		if (ret == -1 || errno == ETIMEDOUT)
		{
			ret = Sck_ErrTimeOut;
			printf("func sckClient_rev() timeout , err:%d \n", Sck_ErrTimeOut);
			return ret;
		} else
		{
			printf("func sckClient_rev() timeout , err:%d \n", Sck_ErrTimeOut);
			return ret;
		}
	}

	int netdatalen = 0;
	ret = readn(connfd, &netdatalen, 4); //read the first 4 bytes, got the ideal length want to read
	if (ret == -1)
	{
		printf("func readn() err:%d \n", ret);
		return ret;
	} else if (ret < 4)
	{
		ret = Sck_ErrPeerClosed;
		printf("func readn() err peer closed:%d \n", ret);
		return ret;
	}

	int n;
	n = ntohl(netdatalen);
	ret = readn(connfd, out, n); //read the data accoding to the length
	if (ret == -1)
	{
		printf("func readn() err:%d \n", ret);
		return ret;
	} else if (ret < n)
	{
		ret = Sck_ErrPeerClosed;
		printf("func readn() err peer closed:%d \n", ret);
		return ret;
	}

	*outlen = n;

	return 0;
}

// client env release
int sckClient_destroy(void *handle)
{
	if (handle != NULL)
	{
		free(handle);
	}
	return 0;
}

int sckCliet_closeconn(int connfd)
{
	if (connfd >= 0)
	{
		close(connfd);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
//function define

//server init
/*
 * func name：sckServer_init
 * description：server's socket init
 * param： port bind port number
 *       listenfd  listen fd
 * success: return 0
   fail: < 0 or the bytes had sent successful
 * */
int sckServer_init(int port, int *listenfd)
{
	int ret = 0;
	int mylistenfd;
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    	//return a new socketfd
	mylistenfd = socket(PF_INET, SOCK_STREAM, 0);
	if (mylistenfd < 0)
	{
		ret = errno;
		printf("func socket() err:%d \n", ret);
		return ret;
	}

	int on = 1;
	ret = setsockopt(mylistenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));//reuse the port
	if (ret < 0)
	{
		ret = errno;
		printf("func setsockopt() err:%d \n", ret);
		return ret;
	}

	ret = bind(mylistenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (ret < 0)
	{
		ret = errno;
		printf("func bind() err:%d \n", ret);
		return ret;
	}

	ret = listen(mylistenfd, SOMAXCONN);
	if (ret < 0)
	{
		ret = errno;
		printf("func listen() err:%d \n", ret);
		return ret;
	}

	*listenfd = mylistenfd;

	return 0;
}

/**
 * accept_timeout - timeout detect accept
 * @fd: socket
 * @addr: output the client addr
 * @wait_seconds: timeout time, 0 is not wait
 * success: return the socket 
   fail: retturn -1 and errno = ETIMEDOUT
 */
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
	int ret;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (wait_seconds > 0)
	{
		fd_set accept_fdset;
		struct timeval timeout;
		FD_ZERO(&accept_fdset);
		FD_SET(fd, &accept_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			ret = select(fd + 1, &accept_fdset, NULL, NULL, &timeout);
		} while (ret < 0 && errno == EINTR);
		if (ret == -1)
			return -1;
		else if (ret == 0)
		{
			errno = ETIMEDOUT;
			return -1;
		}
	}

	//once detected there is a select happened, it means peers has established the three handshake
	if (addr != NULL)
		ret = accept(fd, (struct sockaddr*) addr, &addrlen); //return the established socket
	else
		ret = accept(fd, NULL, NULL);
	if (ret == -1)
	{
		ret = errno;
		printf("func accept() err:%d \n", ret);
		return ret;
	}

	return ret;
}
/*
 * func name：sckServer_accept
 * description：server waiting for the data
 * parm： listenfd listened sock
 * 	     timeout 
 * success: 0
   fail: < 0 or bytes had read
 * */

int sckServer_accept(int listenfd, int *connfd, int timeout)
{
	int ret = 0;
    //
	ret = accept_timeout(listenfd, NULL, (unsigned int) timeout);
	if (ret < 0)
	{
		if (ret == -1 && errno == ETIMEDOUT)
		{
			ret = Sck_ErrTimeOut;
			printf("func accept_timeout() timeout err:%d \n", ret);
			return ret;
		} else
		{
			ret = errno;
			printf("func accept_timeout() err:%d \n", ret);
			return ret;
		}
	}

	*connfd = ret;
	return 0;
}
//server send msg
/*
 * func name：sckServer_send
 * description：send the package and handled the stick package
 * parm： connfd established sock
 * 	     data send package, repack
 * 	     datalen length want to send
 * 	     timeout 
 * success: 0
   fail: < 0 or bytes had read
 * */
int sckServer_send(int connfd, unsigned char *data, int datalen, int timeout)
{
	int ret = 0;
    //write_timeout detect
	ret = write_timeout(connfd, timeout);
	if (ret == 0)
	{
		int writed = 0;
		
		unsigned char *netdata = (unsigned char *) malloc(datalen + 4);
		if (netdata == NULL)
		{
			ret = Sck_ErrMalloc;
			printf("func sckServer_send() mlloc Err:%d\n ", ret);
			return ret;
		}
		int netlen = htonl(datalen);
		memcpy(netdata, &netlen, 4);
		memcpy(netdata + 4, data, datalen);
        //send data
		//writed is the success data length 
		writed = writen(connfd, netdata, datalen + 4);
		
		if (writed < (datalen + 4))
		{
			//release mem
			if (netdata != NULL)
			{
				free(netdata);
				netdata = NULL;
			}
			return writed;
		}

	}
    //timeout
	if (ret < 0)
	{
		if (ret == -1 && errno == ETIMEDOUT)
		{
			ret = Sck_ErrTimeOut;
			printf("func sckServer_send() mlloc Err:%d\n ", ret);
			return ret;
		}
		return ret;
	}

	return ret;
}
//server rev msg
/*
 * func name：sckServer_rev
 * description：receive msg and deal with the stick package
 * parm： connfd established sock
 * 	     out the data had read. 
 * 	     outlen the length had read
 * 	     timeout 
 * */
int sckServer_rev(int connfd, unsigned char *out, int *outlen, int timeout)
{

	int ret = 0;
    
	if (out == NULL || outlen == NULL)
	{
		ret = Sck_ErrParam;
		printf("func sckClient_rev() timeout , err:%d \n", Sck_ErrTimeOut);
		return ret;
	}
	//detect whether read or not.
	ret = read_timeout(connfd, timeout); //bugs modify bombing
	if (ret != 0)
	{
		if (ret == -1 || errno == ETIMEDOUT)
		{
			ret = Sck_ErrTimeOut;
			printf("func sckClient_rev() timeout , err:%d \n", Sck_ErrTimeOut);
			return ret;
		} else
		{
			printf("func sckClient_rev() timeout , err:%d \n", Sck_ErrTimeOut);
			return ret;
		}
	}

	int netdatalen = 0;
	ret = readn(connfd, &netdatalen, 4); 
	if (ret == -1)
	{
		printf("func readn() err:%d \n", ret);
		return ret;
	} else if (ret < 4)
	{
		ret = Sck_ErrPeerClosed;
		printf("func readn() err peer closed:%d \n", ret);
		return ret;
	}
	int n;

	n = ntohl(netdatalen);
	ret = readn(connfd, out, n);
	if (ret == -1)
	{
		printf("func readn() err:%d \n", ret);
		return ret;
	} else if (ret < n)
	{
		ret = Sck_ErrPeerClosed;
		printf("func readn() err peer closed:%d \n", ret);
		return ret;
	}
	*outlen = n;
	return 0;
}

//server env release
int sckServer_destroy(void *handle)
{
     if(handle!=NULL)
     {
    	 free(handle);
    	 handle=NULL;
     }
	return 0;
}

