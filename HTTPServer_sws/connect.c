#include "connect.h"
#include "handle.h"
/*
	FUNC DESCRIPTION:
		This func is used to check should listen what kind of ip addr 
		and then get the remoteip, then handle the request and send 		
		correspond resp information to the client_sock
	PARAMETER:
		int client_sock:
			the socket after success access
		struct sockaddr_storage * client:
			got the sockaddr_storage struct 
			in order to judge that whether belongs v4 or v6
		struct flags * flag:
			in order to give the handler to handle it	
*/
void handle_client_request(int client_sock, struct sockaddr_storage * client, socklen_t client_length, struct flags * flag)
{
	char remoteip[MAX_IP_ADDRESS];
	
	bzero(remoteip, sizeof(remoteip));
	if (client->ss_family == AF_INET6)
	{
		//client ip is ipv6
		if(inet_ntop(AF_INET6, &((struct sockaddr_in6*)client)->sin6_addr, remoteip, sizeof(remoteip)) == NULL)
		{
			perror("inet_ntop error for client v6 address");
		}
	}
	else if(client->ss_family == AF_INET)
	{
		if(inet_ntop(AF_INET, &((struct sockaddr_in *)client)->sin_addr.s_addr, remoteip, sizeof(remoteip)) == NULL)
		{
			perror("inet_ntop error for client v4 address");
		}
	}
	else
	{
<<<<<<< HEAD
		if(strncpy(remoteip, UNKNOWN_IP, sizeof(remoteip) - 1) == NULL)
		{
			perror("remote ip copy err");
			return;
		}		
	
=======
		if(strncpy(remoteip, UNKNOWN_IP, sizeof(remoteip) - 1) == NULL){
			perror("Cannot handle ip");
		}
>>>>>>> d9e1aecf356fee9cc480c282832da9e4b2d622a6
	}
	
	//begin handle the process
	if(handler(client_sock, flag, remoteip) < 0)
	{
		printf("http request err or unknown ip address\n");
		exit(EXIT_FAILURE);
	}
	exit(0);
}


/*
	FUNC DESCRIPTION:
		accept the socket to get the client socket after 
		success finish the three hand-shakes in the kernal
	PARAMETER:
		struct flags * flag:
			as the argument
		int server_socket:
			the bind socket
	RETURN VALUE:
		none
*/
void accept_server_socket(struct flags * flag, int server_socket)
{
	struct sockaddr_storage client;
	int accept_socket;
	pid_t pid;
	socklen_t size;
	
	size = sizeof(struct sockaddr_storage);
	if((accept_socket = accept(server_socket, (struct sockaddr *)&client, &size)) < 0)
	{
		perror("accept error");
		exit(EXIT_FAILURE);
	}
	pid = fork();
	if(pid < 0)
	{
		perror("fork fail");
		exit(EXIT_FAILURE);
	}
	if(pid == 0)
	{
		handle_client_request(accept_socket, &client, sizeof(client), flag);
	}
	if(pid > 0)
	{
		//parent don't need accept client
		if(close(accept_socket) == -1){
			perror("Cannot close for some reason");	
		}
	}
	
}

/*
	FUNC DESCRIPTION:
		This func is used to check whether the client is v4 or v6
		and then bind the ip
	PARAMETER:
		struct flags * flag:
			flag->i_address
	RETURN VALUE:
		> 0 return the server socket
		< 0 exit fail 		
*/
int create_server_socket(struct flags * flag)
{
	//bind(int socket, const struct sockaddr *address, socklen_t address_len);
	int server_sock;
	socklen_t address_len;
	struct sockaddr_in sockaddr_4;
	struct sockaddr_in6 sockaddr_6;
//	struct ifaddrs * ifap0=NULL,*ifap=NULL;
	struct sockaddr * server;

	/*
		if the address is null, it means no -i, listen all ipv4 and ipv6 on the host
	*/
	if(flag->i_address == NULL)
	{
		int ipv6_only = 0;
		/*
		getifaddrs(&ifap0);
		ifap=ifap0;
		if(ifap->ifa_addr->sa_family == AF_INET6)//// check it is IP4
		{
			server_sock = socket(AF_INET, SOCK_STREAM, 0);			
			sockaddr_4.sin_family = AF_INET;
			sockaddr_4.sin_port = htons(flag->p_port);
			sockaddr_4.sin_addr.s_addr = INADDR_ANY;
			address_len = sizeof(sockaddr_4);
			server = (struct sockaddr *)&sockaddr_4;
			
		}//check if it is IP6
		else
		{
			server_sock = socket(AF_INET6, SOCK_STREAM, 0);
			sockaddr_6.sin6_family = AF_INET6;
			sockaddr_6.sin6_addr = in6addr_any;
			sockaddr_6.sin6_scope_id = 0;
			sockaddr_6.sin6_flowinfo = 0;
			sockaddr_6.sin6_port = htons(flag->p_port);
			address_len = sizeof(sockaddr_6);
			server = (struct sockaddr *)&sockaddr_6;
		}
		if (server_sock < 0) 
		{
			err(EXIT_FAILURE, "opening stream socket");
		}
		*/
		server_sock = socket(AF_INET6, SOCK_STREAM, 0);
		if(server_sock < 0)
		{
			err(EXIT_FAILURE, "opening stream socket");
		}
		setsockopt(server_sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&ipv6_only, sizeof(ipv6_only));
		sockaddr_6.sin6_family = AF_INET6;
		sockaddr_6.sin6_addr = in6addr_any;
		sockaddr_6.sin6_scope_id = 0;
		sockaddr_6.sin6_flowinfo = 0;
		sockaddr_6.sin6_port = htons(flag->p_port);
		address_len = sizeof(sockaddr_6);
		server = (struct sockaddr *)&sockaddr_6;
	}
	if((flag->i_address != NULL) && (flag->ipv6 == 0))
	{
		//-i ipv4
		server_sock = socket(AF_INET, SOCK_STREAM, 0);
		if(server_sock < 0)
		{
			err(EXIT_FAILURE, "opening stream socket");
		}
		sockaddr_4.sin_family = AF_INET;
		inet_pton(sockaddr_4.sin_family, flag->i_address, &(sockaddr_4.sin_addr));
		sockaddr_4.sin_port = htons(flag->p_port);
		address_len = sizeof(sockaddr_4);
		server = (struct sockaddr *)&sockaddr_4;
		
	}
	if((flag->i_address != NULL) && (flag->ipv6 == 1))
	{
		//-i ipv6
		sockaddr_6.sin6_family = AF_INET6;
		server_sock = socket(AF_INET6, SOCK_STREAM, 0);
		if(server_sock < 0)
		{
			err(EXIT_FAILURE, "opening stream socket");
		}		
		inet_pton(sockaddr_6.sin6_family, flag->i_address, &(sockaddr_6.sin6_addr));
		sockaddr_6.sin6_scope_id = 0;
		sockaddr_6.sin6_flowinfo = 0;
		sockaddr_6.sin6_port = htons(flag->p_port);
		address_len = sizeof(sockaddr_6);
		server = (struct sockaddr *)&sockaddr_6;
	}
	if (bind(server_sock, server, address_len) < 0)
	{
		printf("binding server socket error\n");
		exit(EXIT_FAILURE);
	}
	if(getsockname(server_sock, server, &address_len) < 0)
	{
		printf("This socket is broken \n");
		exit(EXIT_FAILURE);
	}
	return server_sock;
		
}

/*
	FUNC DESCRIPTION:
		This func is used to run the server and 		
		then close after interactive
	PARAMETER:
		struct flags * flag:
			as the argument
			and check if the server have specific th -d
	RETURN VALUE:
		none
*/
void run_server(struct flags * flag)
{
	int server_sock;
	
	server_sock = create_server_socket(flag);//create and listen
	if(signal(SIGCHLD, handle) == SIG_ERR)
	{
		perror("catch died child fail");
		exit(EXIT_FAILURE);	
	}
	//listen the socket
	if(listen(server_sock, BACKLOG) != 0)
	{
		perror("listen socket fail");
		exit(EXIT_FAILURE);
	}
	
	//daemonize expected the -d

	if (!flag->dflag)
	{
		if (daemon(1, 1) < 0)
		{
			perror("daemon fail");
			exit(EXIT_FAILURE);
		}
	}

	while(1)
	{
		accept_server_socket(flag, server_sock);
	}

	close(server_sock);

	
}
