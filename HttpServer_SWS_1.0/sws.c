#include "util.h"
#include "connect.h"

int main(int argc, char *argv[])
{
	struct flags flag;
	int command;	

	flags_init(&flag);
	while((command = getopt(argc, argv, PARAMETER_FORMAT)) != -1)//process the command
	{
		switch(command)
		{
			case 'd':
				flag.dflag = 1;//using -d
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);//stdlib.h
				break;
			case 'c':
				flag.c_dir = optarg;
				if(isDir(flag.c_dir))				
				{
					printf("invalid CGI dir\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'i':
				flag.i_address = optarg;
				struct in_addr addr4;
				struct in6_addr addr6;
				
				//if the i_address is v4
				if(inet_pton(AF_INET, flag.i_address, &addr4) == 1)//convert IPv4 and IPv6 addresses from text to binary form
				{
					flag.ipv6 = 0;
				}
				//if the i_address is v6
				else if(inet_pton(AF_INET6, flag.i_address, &addr6) == 1)
				{
					flag.ipv6 = 1;
				}
				else
				{
					printf("Neither valid IPV4 or IPV6 address of %s\n", flag.i_address);
					exit(EXIT_FAILURE);
				}
				break;
			case 'l':
				flag.lflag = 1;
				flag.l_file = optarg;
				if((flag.logfd = open(flag.l_file, O_CREAT | O_APPEND | O_WRONLY, 0666)) < 0)
				{
					perror("logfile error");
					exit(EXIT_FAILURE);
				}
				break;
			case 'p':
				flag.p_port = atoi(optarg);
				if ((flag.p_port < MIN_PORT_NUM) || (flag.p_port > MAX_PORT_NUM))
				{
					printf("port number must between %d and %d\n", MIN_PORT_NUM, MAX_PORT_NUM);
					exit(EXIT_FAILURE);
				}
				break;
			default:
				usage();
				exit(EXIT_FAILURE);
				break;
		}
	}
	argc -= optind;
	argv += optind;
	
	
	//the last directory request
	if (argc != 1)
	{
		usage();
		exit(EXIT_FAILURE);
	}
	flag.dir = argv[0];
	if(isDir(flag.dir))
	{	
		printf("invalid directory\n");
		exit(EXIT_FAILURE);
	}	
	
	run_server(&flag);
	close(flag.logfd);
	
	return EXIT_SUCCESS;
}
