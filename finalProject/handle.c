#include "handle.h"

/*
	FUNC DESCRIPTION:
		This function is used to handle all things after get the socket 
		from client and according to the server's flag parameter.
	PARAMETER:
		int client_socket:
			the client socket 
		struct flags * flag:
			the server's flag parameter
		const char * remoteid:
			the client ip in order to write to the log file
	RETURN VALUE:
		> 0 fail
		= 0 success		
*/
int handler(int client_socket, struct flags * flag, const char * remoteid)
{
	struct logging logger;
	struct response responser;
	struct request requester;
	char buf[BUFSIZE];
	char realpath_str[PATH_MAX + 1];
	int header_parsing_failed = 0;
	int cgi_request = 0;
	int serve_file = 0;
	char * token[BUFSIZE];
	int tokenCount = 0;
	int bytes_read;
	int left_length;
	int if_modified_since;
	int content_length;
	int content_type;
	int http_status = 0;
	time_t current_time;
	char * request_line;
	int simple_request = 0;
	char * conditional_request_line;
	//char cgi_output[4096];

	left_length = sizeof(buf) - 1;
	bzero(buf, sizeof(buf));
	request_init(&requester);
	logging_init(&logger);
	
	
	do
	{
		waiting_request(client_socket);//return void
		//buf + (sizeof(buf) - 1 - left_length) means that the steps of pointer moved
		if((bytes_read = read(client_socket, buf + (sizeof(buf) - 1 - left_length), left_length)) < 0)
		{
			perror("read from client socket");
			return -1;
		}
		if(left_length == 0)
		{
			break;
		}
		left_length -= bytes_read;
		
	/*
		if the client type one enter, it will continue waiting for client type again
		if the client type twice enter at one time, it will end read from socket.

	*/
	}while(strstr(buf, CRLF CRLF) == NULL);

	if(strstr(buf, CRLF CRLF) == NULL)//it's up to the max input length but no double ENTER
	{
		response_init(&responser, RESPONSE_STATUS_BAD_REQUEST);
		//int send_html_page(struct response * response, int simple_formate, int client_socket, char * client_msg);
		return send_html_page(&responser, 0, client_socket, NULL);
	}
	else
	{
		if_modified_since = strlen(IF_MODIFIED_SINCE_PREFIX);
		content_length = strlen(CONTENT_LENGTH_PREFIX);
		content_type = strlen(CONTENT_TYPE_PREFIX);
		current_time = time(NULL);
		int failure_status = 0;
		int ret;
		
		request_line = strtok(buf, CRLF);//get the first line of request like: GET /
		if(request_line == NULL)
		{
			perror("there is nothing on the request line");
			return -1;
		}
		//put all these information into the logging struct
		if(strncpy(logger.remoteip, remoteid, sizeof(logger.remoteip) - 1) == NULL)
		{
				perror("remoteip copy err on handler");
				return -1;
		}
		
		if(strncpy(logger.request_lineq, request_line, sizeof(logger.request_lineq) - 1) == NULL)
		{
				 perror("requestline copy err on handler");
				 return -1;
		}
		//int time_to_server_date(time_t *time, char * buf, size_t buf_length);
		if((ret = (time_to_server_date(&current_time, 
		logger.request_time, sizeof(logger.request_time) - 1))) != 0)
		{
			perror("convrt time to the logger");
			return -1;
		}
		conditional_request_line = strtok(NULL, CRLF);//got the second line of request, like: last modified since
		while(conditional_request_line != NULL)//if the conditional_request_line is NULL means doulbe ENTER
		{
			if(strlen(conditional_request_line) > if_modified_since)
			{
				//compare two strings ignoring case,  it compares only the first n bytes of s1.
				if(strncasecmp(conditional_request_line, 
					IF_MODIFIED_SINCE_PREFIX, if_modified_since) == 0)
				{
					char * value_char = conditional_request_line 
					+ if_modified_since;//pointer point to the end
					int white_space = 0;
					int since_time_length;
					char * real_date;
					if(value_char == NULL)
					{
						break;
					}
					//space
					while (isspace((int)*value_char))
					{
						value_char++;
						white_space++;
					}
					since_time_length = strlen(conditional_request_line) 
					- if_modified_since - white_space + 1;//this is the real time length
					real_date = (char *)alloca(since_time_length);//allocate memory that is automatically freed
					if(real_date == NULL)
					{
						perror("real_date got err on handler");
						return -1;
					}
					strncpy(real_date, conditional_request_line 
					+ if_modified_since + white_space, since_time_length - 1);//copy to real_data mem
					real_date[since_time_length - 1] = 0;//   '/0'
					//int current_data_to_time(const char * date, time_t * dst);
					if(gmt_data_to_time(real_date, &(requester.if_modified_since)) < 0)//stored into the struct
					{
						header_parsing_failed = 1;
						break;
					}
				}
			}
			if(strlen(conditional_request_line) > content_length)
			{
				if(strncasecmp(conditional_request_line, 
				CONTENT_LENGTH_PREFIX, content_length) == 0)
				{
					int white_space = 0;
					char * value_char = conditional_request_line + content_length;
					while(isspace((int)*value_char))
					{
						value_char++;
						white_space++;
					}
					requester.content_length = 
					atoi(&(conditional_request_line[content_length + white_space + 1]));
					if(requester.content_length <= 0)
					{
						perror("requester.content_length atoi err on handler");
						return -1;
					}
				}
			}
			if(strlen(conditional_request_line) > content_type)
			{
				if(strncasecmp(conditional_request_line, 
				CONTENT_TYPE_PREFIX, content_type) == 0)
				{
					int white_space = 0;
					char * value_char = conditional_request_line + content_type;
					while(isspace((int)*value_char))
					{
						value_char++;
						white_space++;
					}
					if(snprintf(requester.content_type, 64, "%s", 
			&(conditional_request_line[content_type + white_space + 1])) < 0)
					{
						perror("snprintf err on handler");
						return -1;
					}
				}
			}
			conditional_request_line = strtok(NULL, CRLF);
		}
		//GET http://www.w3.org/pub/WWW/TheProject.html HTTP/1.0
		token[tokenCount] = strtok(request_line, " ");//GET/HEAD
		tokenCount++;
		while((token[tokenCount] = strtok(NULL, " ")) != NULL)
		{
			tokenCount++;
		}
		//token[2] = HTTP version
		if(tokenCount == 2)
		{
			simple_request = 1;//using http/0.9
		}
		//else 1.0
		if((header_parsing_failed) || ((tokenCount < 3) && (!simple_request)))
		{
			response_init(&responser, RESPONSE_STATUS_BAD_REQUEST);
		}
		else if((strncasecmp(token[tokenCount - 1], HTTP_VERSION, 
		sizeof(HTTP_VERSION)) != 0) && (!simple_request))//not the HTTP/1.0 and HTTP/0.9
		{
			response_init(&responser, RESPONSE_STATUS_VERSION_NOT_SUPPORTED);
		}
		else if(strcasecmp(token[0], "GET") == 0)
		{
			requester.method = REQUEST_METHOD_GET;
			strcpy(requester.path, token[1]);
			//The path result stored into the realpath_str
			if(parseuri(&requester, &http_status, flag, realpath_str, &cgi_request) != 0)
			{
				perror("parseuri err on handler");
				return -1;
			}
			response_init(&responser, http_status);
			if(http_status == RESPONSE_STATUS_OK)
			{		
				strncpy(requester.path, realpath_str, PATH_MAX + 1);
			}
		}
		else if(strcasecmp(token[0], "HEAD") == 0)
		{
			requester.method = REQUEST_METHOD_HEAD;
			strcpy(requester.path, token[1]);
			parseuri(&requester, &http_status, flag, realpath_str, &cgi_request);
			{
				perror("parseuri err on handler");
				return -1;
			}
			response_init(&responser, http_status);
			if(http_status == RESPONSE_STATUS_OK)
			{
				strncpy(requester.path, realpath_str, PATH_MAX + 1);
			}
		}
		else
		{
			response_init(&responser, RESPONSE_STATUS_NOT_IMPLEMENTED);
		}
		//send file GET no CGI status ok
		serve_file = ((requester.method == REQUEST_METHOD_GET) 
		&& (responser.response_code == RESPONSE_STATUS_OK) && !cgi_request);

		/*
			Entity-HEADER-FIELD: GET and HEAD both have
		*/
		if ((responser.response_code == RESPONSE_STATUS_OK) 
		&& ((requester.method == REQUEST_METHOD_GET)  
		|| (requester.method == REQUEST_METHOD_HEAD)) && (!cgi_request))
		{
			if(set_entity_body_headers(&responser, realpath_str) < 0)
			{
				perror("set_entity_body_headers err on handler");
				return -1;
			}
			
		}
		if(responser.response_code == RESPONSE_STATUS_OK)
		{
			if(!serve_file)
			{
				/*
				TWO possibles:
					1.using the HEAD. Even if using the HEAD, there also should have the 
						head respond
					2. cgi. Even if using the cgi, there also should have the head respond
				*/

				//if simple_request == 1, the header would send nothing,only for HTTP/0.9
				if(cgi_request)
				{
					if(set_entity_body_headers(&responser, realpath_str) < 0)
					{
						perror("set_entity_body_headers err on handler");
						return -1;
					}
					
				}
				
				failure_status = resp(&responser, client_socket, !simple_request);
				if(failure_status < 0)
				{
					perror("resp err on handler");
					return -1;
				}
			}	
			if(cgi_request) 
			{
				failure_status = execute_cgi(&requester, flag, &http_status, realpath_str, 
							client_socket, &responser);//execute_cgi
				if(failure_status < 0)
				{
					perror("execute_cgi");
					return -1;
				}
			}
			else if(serve_file)
			{
				//GET OK !CGI
				failure_status = fileserver(&requester, &responser, simple_request, client_socket, flag);
				if(failure_status < 0)
				{
					perror("fileserver err on handler");
					return -1;
				}
			}
		}
		else
		{
			//actually, it would send out the bad status resp (header or not header depend on HTTP version)
			failure_status = send_html_page(&responser, simple_request, client_socket, NULL);
			if(failure_status < 0)
			{
				perror("send_html_page err on handler");
				return -1;
			}
		}
		
		strncpy(logger.remoteip, remoteid, sizeof(logger.remoteip) - 1);
		//copy the information to the logger
		snprintf(logger.request_status, sizeof(logger.request_status), "%d", responser.response_code);
		snprintf(logger.response_size, sizeof(logger.response_size), "%d", responser.content_length);
		
		if(flag->dflag)
		{
			if(write_to_log(STDOUT_FILENO, &logger) < 0)
			{
				perror("write_to_log err on handler");
				return -1;
			}
		}
		else if(flag->lflag)
		{
			if(write_to_log(flag->logfd, &logger) < 0)
			{
				perror("write_to_log err on handler");
				return -1;
			}
		}
		return failure_status;
	}
	return 0;
}	


