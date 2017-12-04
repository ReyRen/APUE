#ifndef SWS_HTTP_HANDLE
#define SWS_HTTP_HANDLE

#include "connect.h"

/*
	process the request from client and respond to.
*/
int handler(int client_socket, struct flags * flag, const char * remoteid);


#endif
