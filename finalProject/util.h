#ifndef SWS_HTTP_UTIL
#define SWS_HTTP_UTIL

#include "init.h"

/*
	checking whether the giving is dir
*/
int isDir(const char * dir);
/*
	usage function
*/
void usage(void);
/*
	process died child
*/
void handle(int signum);
/*
	waitting for the client send request data, if timeout
	will respond the timeout infomation
*/
void waiting_request(int client_socket);
/*
	respond function for differe types of respond	
*/
int resp(struct response * response, int client_socket, int full_response);
/*
	respond formate convers and respond the status code and server name
*/
int write_buffer(char * buf, size_t buf_size, const char * format, ...);
/*
	respond with headers 
*/
int resp_header(struct response * response, int client_socket);
/*
	the current timestmp in GMT
*/
int time_to_server_date(time_t *time, char * buf, size_t buf_length);
/*
	put the respond information to the html msg and send to the client
*/
int send_html_page(struct response * response, int simple_formate, int client_socket, char * client_msg);
/*
	put the current time data (char *) to the time_t's time
*/
int gmt_data_to_time(const char * date, time_t * dst);
/*
	Checks that the given URI exist in the system.If the resource exist, it checks the access permission and if it is
	within the allowed directory for serving files or CGI scripts
*/
int parseuri(struct request * request, int * uri_status, struct flags * flag, char * realpath_str, int * cgi_request);
/*
	check there if exist a index.html
*/
int check_index_html(const char * path, char * index_html);
/*
	set to the respond like content type, contnt lenth
*/
int set_entity_body_headers(struct response * response, const char * path);
/*
	got the file type
*/
int fileserver(struct request * request, struct response * response, int simple_response, int socket, struct flags * flag);
/*
	change the file time to the gmtime and transfer to the seconds since from 1900
*/
time_t local_to_gmtime(time_t * time);
/*
	list the directory entries
*/
int send_directory_listing(struct request * request, int socket);
/*
	cgi execute function
*/
int execute_cgi(struct request * requester, struct flags * flag, int * uri_status, char * cgi_path, int socket, struct response * responser);
/*
	write to the log function
*/
int write_to_log(int fd, struct logging * log);
/*
	if the directory not include the index.html, so list the directory's content
*/
int directory_list(struct request * requester, int socket);
#ifdef _MIME_TYPE_
/*
	got the type of file
*/
void mime_type(const char * path, char * dst, size_t dst_len);
#endif
#ifdef _Sun_MIME_TYPE_
/*
	got the type of the file from SunOS
*/
void mime_type_SunOS(const char * path, char * dst, size_t dst_len);
#endif

#endif
