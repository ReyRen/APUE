#ifndef _SCK_CLINT_H_
#define _SCK_CLINT_H_
#ifdef __cplusplus
extern 'C'
{
#endif
	//err define
	#define Sck_Ok	0
	#define Sck_BaseErr	3000
	#define Sck_ErrParam    (Sck_BaseErr+1)
	#define Sck_ErrTimeOut	(Sck_BaseErr+2)
	#define Sck_ErrPeerClosed	(Sck_BaseErr+3)
	#define Sck_ErrMalloc	(Sck_BaseErr+4)

	//define the function

	//client init:
	int sckCliet_init(void **handle,  int contime, int sendtime, int revtime, int nConNum);
	//client connect:
	int sckCliet_getconn(void *handle, char *ip, int port, int *connfd);
	//client close:
	int sckCliet_closeconn(int connfd);
	//client send msg:
	int sckClient_send(void *handle, int  connfd,  unsigned char *data, int datalen);
	//client rcv msg:
	int sckClient_rev(void *handle, int  connfd, unsigned char *out, int *outlen); //1
	//client release env: 
	int sckClient_destroy(void *handle);

	//server init:
	int sckServer_init(int port, int *listenfd);
	//server accept:
	int sckServer_accept(int listenfd, int *connfd,  int timeout);
	//server send msg:
	int sckServer_send(int connfd,  unsigned char *data, int datalen, int timeout);
	//server rcv msg:
	int sckServer_rev(int  connfd, unsigned char *out, int *outlen,  int timeout); //1
	//server release env: 
	int sckServer_destroy(void *handle);

#ifdef __cpluspluse
}
#endif
#endif

