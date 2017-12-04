
#include "init.h"

/*
	initializes the flags and parameters to default values.
*/
void flags_init(struct flags * flag)
{
	flag->ipv6 = 0;
	flag->dflag = 0;
	flag->lflag = 0;
	flag->p_port = DEFAULT_PORT_NUM;
	flag->logfd = 0;
	
	flag->i_address = NULL;
	flag->c_dir = NULL;
	flag->l_file = NULL;
	flag->dir = NULL;
}

/*
	initializes the logging, and bzero memory.
*/
void logging_init(struct logging * logger)
{
	bzero(logger->remoteip, sizeof(logger->remoteip));
	bzero(logger->request_time, sizeof(logger->request_time));
	bzero(logger->request_lineq, sizeof(logger->request_lineq));
	bzero(logger->request_status, sizeof(logger->request_status));
	bzero(logger->response_size, sizeof(logger->response_size));
}

/*
	initializes the response struct, code is given the specific value
*/
void response_init(struct response * responser, int code)
{
	responser->response_code = code;
	responser->last_modified = -1;
	responser->content_length = 0;
	bzero(responser->content_type, sizeof(responser->content_type));
}

/*
	initializes the request struct
*/
void request_init(struct request * request)
{
	request->method = -1;
	request->if_modified_since = -1;
	request->content_length = -1;
	bzero(request->path, sizeof(request->path));
	bzero(request->content_type, sizeof(request->content_type));
	bzero(request->query_string, sizeof(request->query_string));
}

