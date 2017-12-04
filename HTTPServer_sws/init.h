#ifndef SWS_HTTP_INIT
#define SWS_HTTP_INIT

#include <err.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MIME_TYPE_
#include <magic.h>
#endif
#include <getopt.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
//#define _XOPEN_SOURCE
#include <time.h>
#include <ifaddrs.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#ifdef _ALLOCA_TYPE_
#include <alloca.h>
#endif
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <paths.h>
#include <signal.h>
#include <stdarg.h>
#include <libgen.h>
#include <pwd.h>
#include <strings.h>

//macro
#define BUFSIZE 4096


#define PARAMETER_FORMAT "dhc:i:l:p:"

#define MIN_PORT_NUM 1
#define MAX_PORT_NUM 65535
#define DEFAULT_PORT_NUM 8080

#define CRLF "\r\n"
#define HTTP_VERSION "HTTP/1.0"
#define SERVER_VERSION "sws/1.0"
#define CGI_PREFIX "/cgi-bin/"
#define INDEX_HTML "index.html"
#define IF_MODIFIED_SINCE_PREFIX "If-Modified-Since:"
#define CONTENT_LENGTH_PREFIX "Content-Length:"
#define CONTENT_TYPE_PREFIX "Content-Type:"
#define REQUEST_METHOD_GET 1
#define REQUEST_METHOD_HEAD 2
//#define LOGIN_NAME_MAX 10

#define CLIENT_TIMEOUT_SEC 30
#define UNKNOWN_IP "Unknown IP Address"
#define BACKLOG 50
#define FULL_RESPOND 1
#define SIMPLE_RESPOND 0
#define log_request_status 16
#define log_response_size 64
#define method_length 64
#define content_length_cgi 64
#define buf_left_max 128

#define content_type_length 64
#define query_length 255
#define str_len_add_most 8
#define home_dir_sws 4

#define MAX_IP_ADDRESS 64
#define MAX_RECEIVED_TIME_LENGTH 128

//request info
struct request
{
	int method;//GET/HEAD
	char path[PATH_MAX + 1];//request URI
	time_t if_modified_since;//request file time since modifed
	int content_length;//content_length field  for cgi request
	char content_type[content_type_length];//content_type field for cgi request
	char query_string[query_length]; //for cgi GET
};

//HTTP responde code
enum response_status_codes
{
	RESPONSE_STATUS_OK = 200,
	RESPONSE_STATUS_BAD_REQUEST = 400,
	RESPONSE_STATUS_FORBIDDEN = 403,
	RESPONSE_STATUS_NOT_FOUND = 404,
	RESPONSE_STATUS_NOT_IMPLEMENTED = 501,
	RESPONSE_STATUS_VERSION_NOT_SUPPORTED = 505,
	RESPONSE_STATUS_CONNECTION_TIMED_OUT = 522,
	RESPONSE_STATUS_INTERNAL_SERVER_ERROR = 500
};

//response info
struct response
{
	int response_code;
	time_t last_modified;//last modified field
	char content_type[content_type_length];//content type field
	int content_length;//The size in bytes of the data returned.
};

//logging info
struct logging
{
	char remoteip[MAX_IP_ADDRESS];//the remote IP address
	char request_time[MAX_RECEIVED_TIME_LENGTH];//the time the request was received.(int GMT)
	char request_lineq[BUFSIZE];//the first line of request
	char request_status[log_request_status];//the status of request
	char response_size[log_response_size];//size of respond in bytes
};

//flags info
struct flags
{
	int ipv6;//determin whether using the ipv4 or v6
	int dflag;//-d
	int lflag;//-l
	unsigned int p_port;//-p port
	int logfd;//log file descriptor

	const char *i_address;//-i address
	const char *c_dir;//-c dir
	const char *l_file;//-l file
	const char *dir;//the last dir parameter	
};

void flags_init(struct flags * flag);
void logging_init(struct logging * logging);
void response_init(struct response * response, int code);
void request_init(struct request * request);

#endif
