#ifndef SWS_HTTP_CONNECT
#define SWS_HTTP_CONNECT

#include "util.h"

/*Starts the server and transits into daemon mode, if not in debug mode.
  Child is forked when a client connects.
*/
void run_server(struct flags * flag);
/*
	checking using ipv4 to create the socket or ipv6
*/
int create_server_socket(struct flags * flag);
/*
	accept client(got the server_socket from listen queue)
*/
void accept_server_socket(struct flags * flag, int server_socket);
/*
	after accept got the client socket, and handle it(waiting for request...)
*/
void handle_client_request(int client_sock, struct sockaddr_storage * client, socklen_t client_length, struct flags * flag);

#endif
