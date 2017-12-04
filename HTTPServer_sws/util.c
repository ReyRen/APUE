#include "util.h"

/*
        FUNC DESCRIPTION:
  		This function is used to judge the specific parameter 
		if is a directory              
        PARAMETER:
                onst char * dir:
			specific String parameter
        RETURN VALUE:
                > 0 fail or not a directory
		= 0 success is a directory
*/
int isDir(const char * dir)
{
            
        struct stat st;
            
        if (dir == NULL)
        {   
                (void)printf("You provided directory is null\n");
                return 1;
        }   
        if(stat(dir, &st) < 0)
        {   
                (void)printf("cannot open the directory %s\n", dir);
                return 1;
        }   
        if(!S_ISDIR(st.st_mode))
        {   
                (void)printf("%s you provided is not a directory\n", dir);
                return 1;
        }    
            
        return 0;
}

//just print out the usage information to the server stdout
void usage(void)
{
	(void)fprintf(stderr, "usage: ./sws [-dh] [-c dir] [-i address] [-l file] [-p port] dir\n");
}
//handle the died chlid avoid the Zombie process
void handle(int signum)
{
	int pid = 0;
	//printf("recv signum:%d \n", signum);
	while ((pid = waitpid(-1, NULL, WNOHANG) ) < 0)
	{
		perror("wait");
	} 
}
/*
	FUNC DESCRIPTION:
		waiting 20s for client input something into the socket,
		and then the server socket set would get changed. Or, it
		will timeout.
	PARAMETER: 
		int client_socke:
			the accept_socket fd got after accept from server
	RETURN VALUE:
		void
*/
void waiting_request(int client_socket)
{
	fd_set request_read_fd;
	struct timeval tv;
	struct response responser;
	int ret;

	FD_ZERO(&request_read_fd);
	FD_SET(client_socket, &request_read_fd);
	
	tv.tv_sec = CLIENT_TIMEOUT_SEC;//20s
	tv.tv_usec = 0;

	do
	{
		//if encounter the ENITR, it caused by the outer signal interrupted, so continue
		if((ret = select(client_socket + 1, &request_read_fd, NULL, NULL, &tv)) == -1){
			perror("Cannot getting the request");
		}
	} while (ret < 0 && errno == EINTR);
	if(ret < 0)
	{
		perror("select error");
		exit(EXIT_FAILURE);
	}
	else if(ret == 0)
	{
		//timeout
		response_init(&responser, RESPONSE_STATUS_CONNECTION_TIMED_OUT);
		if(resp(&responser, client_socket, FULL_RESPOND) < 0)
		{
			perror("resp err on waiting_request");
			return;
		}
	}
}

/*
        FUNC DESCRIPTION:
  		respond to client with status line and header lines(call the resp_header func).
		if choose not full_response, there would not respone both
		of these mentioned above. 	              
        PARAMETER:
                struct response * response:
			struct of response
		int client_socket:
			the accept_socket fd got after accept from server
		int full_response:
			the flag to veriify whether send the header only or 
			together with the body.
        RETURN VALUE:
                < 0 perror
		= 0 success
*/
int resp(struct response * responser, int client_socket, int full_response)
{
	int code;
	char buf[BUFSIZE];
	size_t buf_size;
	int write_length;
	int header_result;

	if(!full_response)
	{
		return 0;
	}

	code = responser->response_code;
	buf_size = sizeof(buf);
	
	switch(code)
	{
		case RESPONSE_STATUS_OK:
			write_length = write_buffer(buf, buf_size, "%s %d OK%s", HTTP_VERSION, code, CRLF);
			break;			
		case RESPONSE_STATUS_BAD_REQUEST:
			write_length = write_buffer(buf, buf_size, "%s %d BAD REQUEST%s", HTTP_VERSION, code, CRLF);
			break;
		case RESPONSE_STATUS_FORBIDDEN:
			write_length = write_buffer(buf, buf_size, "%s %d FORBIDDEN%s", HTTP_VERSION, code, CRLF);
			break;
		case RESPONSE_STATUS_NOT_FOUND:
			write_length = write_buffer(buf, buf_size, "%s %d NOT FOUND%s", HTTP_VERSION, code, CRLF);
			break;
		case RESPONSE_STATUS_NOT_IMPLEMENTED:
			write_length = write_buffer(buf, buf_size, "%s %d NOT IMPLEMENTED%s", HTTP_VERSION, code, CRLF);
			break;
		case RESPONSE_STATUS_VERSION_NOT_SUPPORTED:
			write_length = write_buffer(buf, buf_size, "%s %d VERSION NOT SUPPORTED%s", HTTP_VERSION, code, CRLF);
			break;
		case RESPONSE_STATUS_CONNECTION_TIMED_OUT:
			write_length = write_buffer(buf, buf_size, "%s %d CONNECTION TIMED OUT%s", HTTP_VERSION, code, CRLF);
			break;
		default:
			code =  RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
			write_length = write_buffer(buf, buf_size, "%s %d INTERNAL SERVER ERROR%s", HTTP_VERSION, code, CRLF);
			break;
	}
	if(write_length < 0)//actual size of wrote to the buf
	{
		perror("failed to write to buffer");
		return -1;
	}
	if(write(client_socket, buf, write_length) < 0)
	{
		perror("write to socket fail");
		return -1;
	}
	//send respond header to client
	header_result = resp_header(responser, client_socket);
	if(code == RESPONSE_STATUS_CONNECTION_TIMED_OUT)
	{
		//printf("connect time out\n");
		close(client_socket);
		exit(EXIT_SUCCESS);//exit the client process
	}
	return header_result;	
}
/*
	FUNC DESCRIPTION:
		using the variable argument convert to the format data
*/
int write_buffer(char * buf, size_t buf_size, const char * format, ...)
{
        va_list args;
        int write_length;

        va_start(args, format);//variable argument lists
        //int vsprintf(char *str, const char *format, va_list ap);
        write_length = vsnprintf(buf, buf_size, format, args);//formatted output conversion
        va_end(args);
        if ((write_length < 0) || (write_length >= buf_size))
        {   
                return -1; 
        }   
        else
        {   
                return write_length;
        }   
}

/*
        FUNC DESCRIPTION:
                respond: date, server, last-modified, content-type, content-length
       	PARAMETER:
               	struct response * response:
			struct of response
		int client_socket:
			the accept_socket fd got after accept from server
        RETURN VALUE:
                < 0 perror
		= 0 success
*/
int resp_header(struct response * responser, int client_socket)
{
	char buf[BUFSIZE];
	time_t tm;
	char server_time[buf_left_max];
	char last_modified_time[buf_left_max];
	int ret;
	size_t buf_size;
	int write_length;
	char * bufP;

	buf_size = sizeof(buf);
	bufP = buf;
	
<<<<<<< HEAD
	tm = time(NULL);//get current time
	if(tm < 0)
	{
		perror("get current time err");
		return -1;
=======
	/* get current time */
	if((tm = time(NULL)) == -1){
		perror("Cannot get current time");
>>>>>>> d9e1aecf356fee9cc480c282832da9e4b2d622a6
	}
	ret = time_to_server_date(&tm, server_time, sizeof(server_time));
	if(ret < 0)
	{
		printf("get currrent server date fail\n");
		return -1;
	}

	//date
	write_length = write_buffer(bufP, buf_size, "Date: %s%s", server_time, CRLF);
	if(write_length < 0)
	{
		printf("write current server date fail\n");
		return -1;
	}
	buf_size -= write_length;
	bufP += write_length;
	
	//go on
	//server name
	write_length = write_buffer(bufP, buf_size, "Server: %s%s", SERVER_VERSION, CRLF);
	if(write_length < 0)
	{
		printf("write server version fail\n");
		return -1;
	}
	buf_size -= write_length;
	bufP += write_length;

	//go on
	//last_modified
	if(responser->last_modified != -1)
	{
		ret = time_to_server_date(&responser->last_modified, last_modified_time, sizeof(last_modified_time));
		if(ret < 0)
		{
			perror("convert to last_modified time fail");
			return -1;
		}
		write_length = write_buffer(bufP, buf_size, "Last-Modified: %s%s", last_modified_time, CRLF);
		if(write_length < 0)
		{
			(void)printf("write last modified time fail\n");	
			return -1;
		}
		buf_size -= write_length;
		bufP += write_length;
	}
	
	//go on
	//Content-Type
	if(strlen(responser->content_type) > 0)
	{
#ifdef _Sun_MIME_TYPE_
		write_length = write_buffer(bufP, buf_size, "Content-Type: %s%s", responser->content_type, "");
#endif
#ifdef _MIME_TYPE_
		write_length = write_buffer(bufP, buf_size, "Content-Type: %s%s", responser->content_type, CRLF);	
#endif
		if(write_length < 0)
		{
			(void)printf("write content type fail\n");
			return -1;
		}
		buf_size -= write_length;
		bufP += write_length;
	}

	//go on
	//Content-Length
	if(responser->content_length >= 0)
	{
		write_length = write_buffer(bufP, buf_size, "Content-Length: %d%s", responser->content_length, CRLF);
		if(write_length < 0)
		{
			(void)printf("write content length fail\n");
			return -1;
		}
		buf_size -= write_length;
		bufP += write_length;
	}

	//go  on
	//blank line as described in RFC 1945
	write_length = write_buffer(bufP, buf_size, CRLF);
	if(write_length < 0)
	{
		printf("write blank line fail\n");
		return -1;
	}
	buf_size -= write_length;
	bufP += write_length;

	//to socket
	if(write(client_socket, buf, sizeof(buf) - buf_size) < 0)
	{
		perror("write header to socket faill");
		return -1;
	}
	return 0;
}

/*
        FUNC DESCRIPTION:
                This function is used to get the current GMT time and 
		convert to the HTTP/1.0 format.
        PARAMETER:
		time_t * time:
			time_t type
		buf:
			store the converted time format                 
        RETURN VALUE:
              	< 0 perror
	        = 0 success
*/
int time_to_server_date(time_t * time, char * buf, size_t buf_length)
{
	struct tm tmm;

	if(time == NULL)
	{
		return -1;
	}
	if(gmtime_r(time, &tmm) == NULL)
	{
		return -1;
	}
	if(strftime(buf, buf_length, "%a, %d %b %Y %H:%M:%S GMT", &tmm) == 0)
	{
		return -1;
	}
	return 0;
}

/*
        FUNC DESCRIPTION:
  		This function is used to send the respond msg to client. According the request line's 
		mode and reference the RFC 1945 to respond the proper msg. 
        PARAMETER:
		struct response * responser:
			struct of response
		int simple_formate:
			if simple_format == 0 means no simple_format so it would be send headers
			or there would be using simple_format, it would not send headers.
			HTTP/0.9 would send the simple_format according RFC 1945
		int client_socket:
			the accept_socket fd got after accept from server
		char * client_msg:
			the respond msg after success parse the request from client
        RETURN VALUE:
                < 0 perror
		= 0 success
*/
int send_html_page(struct response * responser, int simple_formate, int client_socket, char * client_msg)
{	
	int code;
	char message[BUFSIZE];
	char buf[BUFSIZE];
	size_t left_length;
	char * bufP;
	int write_length;

	code = responser->response_code;
	switch(code)
	{
		case RESPONSE_STATUS_OK:
			write_length = write_buffer(message, sizeof(message), "%d - OK", code);
			break;
		case RESPONSE_STATUS_BAD_REQUEST:
			write_length = write_buffer(message, sizeof(message), "%d - BAD REQUEST", code);
			break;
		case RESPONSE_STATUS_FORBIDDEN:
			write_length = write_buffer(message, sizeof(message), "%d - STATUS FORBIDDEN", code);
			break;
		case RESPONSE_STATUS_NOT_FOUND:
			write_length = write_buffer(message, sizeof(message), "%d - FILE NOT FOUND", code);
			break;
		case RESPONSE_STATUS_NOT_IMPLEMENTED:
			write_length = write_buffer(message, sizeof(message), "%d - NOT IMPLEMENTED", code);
			break;
		case RESPONSE_STATUS_VERSION_NOT_SUPPORTED:
			write_length = write_buffer(message, sizeof(message), "%d - VERSION NOT SUPPORTED", code);
			break;
		case RESPONSE_STATUS_CONNECTION_TIMED_OUT:
			write_length = write_buffer(message, sizeof(message), "%d - CONNECTION TIMED OUT", code);
			break;
		case RESPONSE_STATUS_INTERNAL_SERVER_ERROR:
			write_length = write_buffer(message, sizeof(message), "%d - INTERNAL SERVER ERROR", code);
			break;
		default:
			write_length = write_buffer(message, sizeof(message), "%d - UNKNOWN ERROR", code);
			break;
	}
	
	if(write_length < 0)
	{
		perror("write to the buf");
		return -1;
	}
	
	//implement html respond
	left_length = sizeof(buf);
	bufP = buf;
	
	write_length = write_buffer(bufP, left_length, "<html>%s<head>%s", CRLF, CRLF);
	if(write_length < 0)
	{
		perror("fail to write html to buffer");
		return -1;
	}
	left_length -= write_length;
	bufP += write_length;
	
	write_length = write_buffer(bufP, left_length, "<title>%s</title>%s</head>%s", message, CRLF, CRLF);
	if(write_length < 0)
	{
		perror("fail to write html to buffer");
		return -1;
	}
	left_length -= write_length;
	bufP += write_length;
	
	write_length = write_buffer(bufP, left_length, "<body>%s<h1>GROUP5_SWS/1.0</h1>%s", CRLF, CRLF);
	if(write_length < 0)
	{
		perror("fail to write html to buffer");
		return -1;	
	}
	left_length -= write_length;
	bufP += write_length;
	
	if(client_msg != NULL)
	{
		write_length = write_buffer(bufP, left_length, "<p>%s</p>%s<p>%s</p>%s</body>%s</html>%s", message, CRLF, client_msg, CRLF, CRLF, CRLF);
	}
	else
	{
		client_msg = "No Data Got";
		write_length = write_buffer(bufP, left_length, "<p>%s</p>%s<p>%s</p>%s</body>%s</html>%s", message, CRLF, client_msg, CRLF, CRLF, CRLF);
	}
	if(write_length < 0)
	{
		perror("fail to write html to buffer");
		return -1;
	}
	left_length -= write_length;
	bufP += write_length;
	
	if(resp(responser, client_socket, !simple_formate) != 0)
	{
		perror("send header to client fail");
		return -1;
	}
	if(write(client_socket, buf, sizeof(buf) - left_length))
	{
		perror("write to socket of html fail");
		return -1;
	}
	return 0;
}

/*
	FUNC DESCRIPTION:
		This func is used to conver the request time type to the 
		time_t and then to the ASCII		
	PARAMETER:
		const char * date:
			Got from the clinet request If-Modified_science: 
		time_t * dst:
			time_t to store the convered time		
	RETURN VALUE:
		< 0 convert fail
		= 0 success
*/
int gmt_data_to_time(const char * date, time_t * dst)
{
	//If-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT  from RFC1945	
	//If-Modified_since: Sunday, 06-Nov-94 08:49:37 GMT from RFC 850
	//if-Modified_since: Sun Nov  6 08:49:37 1994 from ASCTime
	//my respond date: Date: Fri, 10 Nov 2017 18:56:14 GMT
	struct tm tmm;
	int week_day_len;
	char * p;
	time_t time;

	if(date == NULL)
	{
		return -1;
	}
	bzero(&tmm, sizeof(tmm));
	if((p = strchr(date, ',')) == NULL)
	{
		//ASCI
		/*		
		if(strptime(date, "%a %b %e %H:%M:%S %Y", &tmm) == NULL)
		{
			return -1;
		}*/
		char *check = strptime(date, "%a %b %e %H:%M:%S %Y", &tmm);
		if(check == NULL || *check != '\0')
		{
			return -1;
		}
	}
	else
	{
		week_day_len = p - date;//actually it's the amount of element
		if(week_day_len == 3)
		{
			//strptime - convert a string representation of time to a time tm structure
			/*if(strptime(date, "%a, %d %b %Y %H:%M:%S GMT", &tmm) == NULL)
			{
				return -1;
			}*/
			/* strptime - convert a string representation of time to a time tm structure */
			char *check = strptime(date, "%a, %d %b %Y %H:%M:%S GMT", &tmm);
			if(check == NULL || *check != '\0')
			{
				return -1;
			}
		}
		else
		{
			char *check = strptime(date, "%A, %d-%b-%y %H:%M:%S GMT", &tmm);
			if(check == NULL || *check != '\0')	
			{
				return -1;
			}
		}
	}
	//transform date and time to broken-down time or ASCII
	if((time = mktime(&tmm)) == -1)
	{
		perror("ASCII time fail");
		return -1;
	}
	*dst = time;
	return 0;
}

/*
	FUNC DESCRIPTION:
		This func is used to parse the uri as the realpath, assemble with
		the server's firectory
	PARAMETER:
		struct request * request:
			request information struct
		int * uri_status:
			in order to return the correspond respond status 
		struct flags * flag:
			flag information struct
		char * realpath_str:
			in order to return the parse result of real path
		int * cgi_request:
			in order to return the cgi flag
	RETURN VALUE:
		< 0 fail
		= 0 success
*/
int parseuri(struct request * request, int * uri_status, struct flags * flag, char * realpath_str, int * cgi_request)
{
	struct passwd * pw;
	char * username;	
	int mode;
	char uri_path[PATH_MAX + 1];
	bzero(uri_path, sizeof(uri_path));
	char uri_real_path[PATH_MAX + 1];
	bzero(uri_real_path, sizeof(uri_real_path));
	char server_real_path[PATH_MAX + 1];
	bzero(server_real_path, sizeof(server_real_path));
	char * chp;

	//home or not
	if(request->path[0] == '/' && request->path[1] == '~')
	{
		//home
		const char * userdir = request->path + 2;// /~jschauma/
		int i;
		for(i = 0; i < strlen(userdir); i++)
		{
			if(userdir[i] == '/')
			{
				break;//got the username
			}
		}
		if(i > LOGIN_NAME_MAX)
		{
			*uri_status = RESPONSE_STATUS_BAD_REQUEST;
			return -1;
		}
		
		if((username = (char *)malloc(i + 1)) == NULL){
			perror("Malloc fail");
		}
			
		if(strncpy(username, userdir, i) == NULL){
			perror("Invild request");
		}
		username[i] = '\0';//end
		
		//struct passwd *getpwnam(const char *name); get password file entry
		if((pw = getpwnam((const char *) username)) == NULL)
		{
			free(username);
			//no this user
			*uri_status = RESPONSE_STATUS_NOT_FOUND;
			return -1;
		}
		free(username);
		
		//char *realpath(const char *path, char *resolved_path); return the canonicalized absolute pathname
		/*
			struct passwd {
               char   *pw_name;       username 
               char   *pw_passwd;      user password 
               uid_t   pw_uid;         user ID
               gid_t   pw_gid;         group ID 
               char   *pw_gecos;       user information 
               char   *pw_dir;         home directory 
               char   *pw_shell;       shell program 
           };
		*/
		//resolved_path is specified as NULL, then realpath() uses malloc(3) to allocate a 
		//buffer of up to PATH_MAX bytes to hold the resolved pathname, and returns a
       		//pointer to this buffer.
		if(realpath(pw->pw_dir, server_real_path) == NULL) //user's home directory  /home/name
		{	
			*uri_status = RESPONSE_STATUS_NOT_FOUND;
			return -1;
		}	
		if(strncat(server_real_path, "/sws", home_dir_sws))
		{
			*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
			return -1;
		}
		//realpath for requested uri
		userdir = userdir + i;// after the home directory, this include the '/'
		if((strlen(server_real_path) + strlen(userdir)) > PATH_MAX)
		{
			*uri_status = RESPONSE_STATUS_BAD_REQUEST;
			return -1;
		}
		//this usi_path is the path after parse /~jschauma to /home/jsch../....
		if((strncpy(uri_path, server_real_path, strlen(server_real_path)) == NULL) || (strncat(uri_path, userdir, strlen(userdir) + 1) == NULL))//  /home/name
		{
			*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
			return -1;
		}
	}else if(strstr(request->path, "/cgi-bin") == request->path && flag->c_dir != NULL)
	{
		if(realpath(flag->c_dir, server_real_path) == NULL)
                {
                        *uri_status = RESPONSE_STATUS_NOT_FOUND;;
                        return -1;
                }
		if(strlen(server_real_path) + strlen(request->path + str_len_add_most) > PATH_MAX)
		{
			*uri_status = RESPONSE_STATUS_BAD_REQUEST;
			return -1;
		}
		*cgi_request = 1;
		/* using the -c dir replace the /cgi-bin */
		if(strncpy(uri_path, server_real_path, strlen(server_real_path) + 1) == NULL){
			perror("Cannot handle request");
		}
		//if the -c dir have the / at the dir's end
		if(*(server_real_path + (strlen(server_real_path) - 1)) == '/')
		{
<<<<<<< HEAD
			//have the / at the end of the -c dir
			strncat(uri_path, request->path + strlen(CGI_PREFIX), strlen(request->path) + 1 - strlen(CGI_PREFIX));
		}
		else
		{
			strncat(uri_path, request->path + (strlen(CGI_PREFIX) - 1), strlen(request->path) - strlen(CGI_PREFIX) + 2);
=======
			/* have the / at the end of the -c dir */
			if(strncat(uri_path, request->path + strlen(CGI_PREFIX), strlen(request->path) + 1 + strlen(CGI_PREFIX)) == NULL){
				perror("Cannot handle request");
			}
		}
		else
		{
			if(strncat(uri_path, request->path + (strlen(CGI_PREFIX) - 1), strlen(request->path) + strlen(CGI_PREFIX)) == NULL){
				perror("Cannot handle request");
			}
>>>>>>> d9e1aecf356fee9cc480c282832da9e4b2d622a6
		}
		chp = uri_path;
		while((*chp != '?') && (*chp != '\0'))
		{
			chp++;
		}
		if(*chp == '?')
		{
			//interrupted 
			*chp = '\0';
			if(strncpy(request->query_string, ++chp, sizeof(request->query_string) - strlen("QUERY_STRING=")) == NULL){
				perror("Cannot handle request");
			}
		}
	}
	else
	{
		//using the server specificed dir
		if(realpath(flag->dir, server_real_path) == NULL)
		{
			*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;//server directory
			return -1;
		}
		if((strlen(server_real_path) + strlen(request->path)) > PATH_MAX)
		{
			*uri_status = RESPONSE_STATUS_BAD_REQUEST;
			return -1;
		}
		if(*(server_real_path + (strlen(server_real_path) - 1)) == '/')
		{
			if(*(request->path) != '/')
			{
				if((strncpy(uri_path, server_real_path, strlen(server_real_path)) == NULL) 
				|| (strncat(uri_path, request->path, strlen(request->path) + 1) == NULL))
				{
					*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
					return -1;
				}
			}
			else
			{
				if((strncpy(uri_path, server_real_path, strlen(server_real_path)) == NULL) 
				|| (strncat(uri_path, (request->path + 1), strlen(request->path)) == NULL))
				{
					*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
					return -1;
				}
			}
		}
		else
		{
			if(*(request->path) != '/')
			{
				if((strncpy(uri_path, server_real_path, strlen(server_real_path)) == NULL) 
				|| (strncat(uri_path, "/", 1) == NULL)
				|| (strncat(uri_path, request->path, strlen(request->path) + 1) == NULL))
				{
					*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
					return -1;
				}
			}
			else
			{
				if((strncpy(uri_path, server_real_path, strlen(server_real_path)) == NULL)
				|| (strncat(uri_path, request->path, strlen(request->path) + 1) == NULL))
				{
					*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
					return -1;
				}
			}
		}
		
	}
	if(*cgi_request)//execute the cgi
	{
		mode = R_OK |  X_OK;//if having the execute and read permission
	}else
	{
		switch(request->method)
		{
			case REQUEST_METHOD_GET:
				mode = R_OK;//check if having the read permission.
				break;
			case REQUEST_METHOD_HEAD:
				mode = R_OK;
				break;
			default:
			  	*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
				return -1;
				break;
						
		}
	}
	if(access(uri_path, mode) != 0)
	{
		switch (errno)
		{
			case EACCES://Permission denied (POSIX.1)
				*uri_status = RESPONSE_STATUS_FORBIDDEN;
				return -1;
				break;
			case EROFS://Read-only filesystem (POSIX.1)
				*uri_status = RESPONSE_STATUS_FORBIDDEN;
				return -1;
				break;
			case ENAMETOOLONG://Filename too long (POSIX.1)
				*uri_status = RESPONSE_STATUS_BAD_REQUEST;
				return -1;
				break;
			case ENOENT://No such file or directory
				*uri_status = RESPONSE_STATUS_NOT_FOUND;
				return -1;
				break;
			case ENOTDIR://Not a directory (POSIX.1)
				*uri_status = RESPONSE_STATUS_NOT_FOUND;
				return -1;
				break;
			default:
				*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
				return -1;
				break;
		}
		return -1;
	}
	if(realpath(uri_path, uri_real_path) == NULL)
	{
		*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
		return -1;
	}
	if(strstr(uri_real_path, server_real_path) != NULL)//check again, if the dir append ok
	{
		*uri_status = RESPONSE_STATUS_OK;
		if(*cgi_request)
		{
			if(strncpy(realpath_str, (char*) uri_real_path, sizeof(uri_real_path)) == NULL){
				perror("Cannot handle URI");
			}
		}else
		{
			check_index_html(uri_real_path, realpath_str);
		}
		return 0;
	}else
	{
		*uri_status = RESPONSE_STATUS_FORBIDDEN;
		return -1;
	}
	return -1;
}

/*
	FUNC DESCRIPTION:
		This func is used to check the uri if a dir, 
		if yes, check if there is a index.html
		or, ok	
	PARAMETER:
		const char * path:
			The parsed uri above
		char * index_html:
			the result of uri (append or not)index.html 
	RETURN VALUE:
		< 0 something wrong
		= 0 success
*/
int check_index_html(const char * path, char * index_html)
{
	struct stat st;
	
	if(stat(path, &st) == -1)
	{
		if((strncpy(index_html, path, PATH_MAX + 1)) == NULL){
			perror("Cannot handle URI");
		}
		return -1;
	}
	if(!S_ISDIR(st.st_mode))
	{
		/* the uri is not a directory. */
		if((strncpy(index_html, path, PATH_MAX + 1)) == NULL){
			perror("Cannot handle URI");
		}
		return 0;
	}
	// is dir
	if(strlen(path) + strlen(INDEX_HTML) < PATH_MAX)
	{
		strncpy(index_html, path, PATH_MAX + 1);
		if(index_html[strlen(index_html) - 1] == '/')
		{
			if((strncat(index_html, INDEX_HTML, 11)) == NULL){
				perror("Cannot handle URI");
			}
		}else
		{
			if((strncat(index_html, "/" INDEX_HTML, 12)) == NULL){
				perror("Cannot handle URI");
			}
		}
	}
	if(stat(index_html, &st) == 0)
	{
		//check that the index.html if is a directory
		if(!S_ISDIR(st.st_mode))
		{
			if(access(index_html, R_OK) == 0)
			{
				return 0;
			}
			else
			{
				if(strncpy(index_html, path, PATH_MAX + 1) == NULL){
					perror("Cannot handle URI");
				}
				return 0;
			}
		}
		else
		{
			/* directory named index.html */
			if((strncpy(index_html, path, PATH_MAX + 1)) == NULL){
				perror("Cannot handle URI");
			}
			return 0;
		}
		
	}
	else
	{
		if(errno == ENOENT)
		{
			//No such file or directory means there is no index.html under that directory
		}
		else
		{
			perror("index.html error");
			return -1;
		}
	}
	if((strncpy(index_html, path, PATH_MAX + 1)) == NULL){
		perror("Cannot handle URI");
	}
	return 0;
}

/*
	FUNC DESCRIPTION:
		This func is used to set the Entity-Header
	PARAMETERS:
		struct response * responser:
			responser struct
		const char * path:
			the parsed uri
	RETURN VALUE:
		< 0 something err
		= 0 success
*/
int set_entity_body_headers(struct response * responser, const char * path)
{
	struct stat st;
		
	if(path == NULL)
	{
		perror("The path is NULL");
		return -1;
	}
	if(stat(path, &st) < 0)
	{
		perror("stat error");
		return -1;
	}
	else
	{
#ifdef _Sun_MIME_TYPE_
		mime_type_SunOS(path, responser->content_type, sizeof(responser->content_type));
#endif
#ifdef _MIME_TYPE_
		mime_type(path, responser->content_type, sizeof(responser->content_type));
#endif
		responser->content_length = st.st_size;
		responser->last_modified = st.st_mtime;
		return 0;
	}
}

#ifdef _Sun_MIME_TYPE_
/*
 *	FUNC DESCRIPTION:
 *		There is no libmagic on the SunOS, so using the 
 *		execv to execute the file command to get the 
 *		content type
 *	PARAMETER:
 *		const char * path:
 *			the real path of the real path
 *		char *dst:
 *			the storage of content_type
 *		dst_len:
 *			used to bzero
 *	RETURN VALUE:
 *		none
 * */
void mime_type_SunOS(const char * path, char *dst, size_t dst_len)
{
	bzero(dst, dst_len);
	pid_t pid;
	int pipe_f[2];
	int status;
	char tmp[query_length];
	int type_length;
	bzero(tmp, sizeof(tmp));
	char * cur_p;

	if(path == NULL)
	{
		return;
	}
	if(pipe(pipe_f) < 0)
	{
		perror("pipe err for SunOS mime");
		return;
	}
	if((pid = fork()) < 0)
	{
		perror("fork err for SunOS mime");
		return;
	}
	if(pid == 0)
	{
		dup2(pipe_f[1], STDOUT_FILENO);
		close(pipe_f[0]);
		execlp("/usr/bin/file", "file", path, (char *)NULL);
		exit(0);
	}
	else
	{
		close(pipe_f[1]);
		type_length = read(pipe_f[0], tmp, sizeof(tmp));
		if(type_length > 0)
		{
			strncpy(dst, tmp, type_length + 1);		
			cur_p = dst;
			while((strncmp(cur_p, ":", 1) != 0))
			{
				cur_p++;
				type_length--;
			}
			cur_p++;
			type_length--;
			while(isspace((int)*cur_p))			
			{
				cur_p++;
				type_length--;
			}
			//bzero(dst, dst_len);
			//memset(dst, 0, dst_len);
			strncpy(dst, cur_p, type_length + 1);
		}
		else
		{
			close(pipe_f[0]);
			if(waitpid(pid, &status, 0) < 0)
			{
				perror("waitpid err on mime for sun");
				return;
			}
			return;
		}
		close(pipe_f[0]);
		if(waitpid(pid, &status, 0) < 0)
		{
			perror("waitpid err on mime for sun");
			return;
		}
	}


	
}
#endif

#ifdef _MIME_TYPE_
/*
	FUNC DESCRIPTION:
		This func is used to got the mime type of uri file
	PARAMETER:
		const char * path:
			this is the uri real path
		char * dst:
			store to the dst after got the mime type
	RETURN VALUE:
		none;
		
*/
void mime_type(const char * path, char * dst, size_t dst_len)
{
	magic_t magic;
	const char * mime_type;
	
	if(path == NULL)
	{
		return;
	}
	//magic_t magic_open(int flags); Return a MIME type string, instead of a textual description.
	if((magic = magic_open(MAGIC_MIME_TYPE)) == NULL)
	{
		perror("magic_open");
		return;
	}
	//int magic_load(magic_t cookie, const char *filename);
	//NULL for the default database file
	if(magic_load(magic, NULL) != 0)
	{
		perror("magic_load");
	}	
	//const char * magic_file(magic_t cookie, const char *filename);
	if((mime_type = magic_file(magic, path)) == NULL)	
	{
		perror("magic_file");
		magic_close(magic);
		return;
	}
	bzero(dst, dst_len);
	strncpy(dst, mime_type, dst_len - 1);
	magic_close(magic);	
}
#endif
/*
	FUNC DESCRIPTION:
		This function is used to send correspondent information (simple or not simple
		cgi or not..) respond information to the client.	
	PARAMETER:
		struct request * requester:
			request informatoin struct
		struct response * responser:
			response information struct
		int simple_response:
			0 is means send as full respond
			1 is means send as simple respond
		int socket:
			client socket
		struct flags * flag:
			the server's arguments struct
	RETURN VALUE:
		< 0 fail
		= 0 success
		
*/
int fileserver(struct request * requester, struct response * responser, int simple_response, int socket, struct flags * flag)
{
  	int fd;
  	struct stat st;
  	int n_bytes;
  	char buf[BUFSIZE];

  	if(stat(requester->path, &st) != 0)
	{
    		perror("stat err");
    		response_init(responser, RESPONSE_STATUS_INTERNAL_SERVER_ERROR);
		/*
			if using the simple_response, it means using HTTP/0.9
			that would just html body get respond.
		*/
    		return send_html_page(responser, simple_response, socket, NULL);
  	}

  	if((requester->if_modified_since != -1) && (requester->if_modified_since >= local_to_gmtime(&st.st_mtime)))
 	{
    		// file is not new enough. 
    		responser->content_length = 0;
    		bzero(responser->content_type, sizeof(responser->content_type));
    		return resp(responser, socket, !simple_response);
  	}

  	if(resp(responser, socket, !simple_response) != 0) //headers 
	{
    		perror("failed to write response headers");
    		return -1;
  	}

  	if(!S_ISDIR(st.st_mode)) 
	{
    		//at least having the read permission
    		if((fd = open(requester->path, O_RDONLY)) < 0)
		{
      			perror("open err");
      			return -1;
    		}

    		while((n_bytes = read(fd, buf, BUFSIZE)) > 0) 
		{
      			if(write(socket, buf, n_bytes) != n_bytes)
 			{
        			perror("write err");
        			return -1;
      			}
    		}

    		if (n_bytes < 0) 
		{
      			perror("read");
      			return -1;
    		}

    		return 0;
  	} 
	else
	{
		//directory
    		if (directory_list(requester, socket) < 0) {
			perror("directory_list err");
      			return -1;
    		}
  	}

  	return 0;
}

/*
	FUNC DESCRIPTION:
		This function is used to transfer the GMT time to 
		the seconds from 1970
	PARAMETER:
		time_t * time:
			time_t types of time
	RETURN VALUE:
		< 0 someting err
		> 0 success
*/
time_t local_to_gmtime(time_t * time)
{
	struct tm result;
	if (gmtime_r(time, &result) == NULL)
	{
		return -1;
	}
	return mktime(&result);
}

/*
	FUNC DESCRIPTION:
		This func is used to display the directory's files after there is no index.html
	PARAMETER:
		struct request * requester:
			the requeste information struct
		int socket:
			client socket to write to
	RETURN VALUE:
		< 0 fail
		= 0 success
*/
int directory_list(struct request * requester, int socket)
{
  	char buf[BUFSIZE];
  	size_t buf_left;
  	int written;
  	char * p;
  	struct dirent ** namelist;
  	int entries;
  	int i;

  	buf_left = sizeof(buf);
  	p = buf; 
	
	/*
	int scandir(const char *dirp, struct dirent ***namelist,
              int (*filter)(const struct dirent *),
              int (*compar)(const struct dirent **, const struct dirent **));
	*/
  	entries = scandir(requester->path, &namelist, 0, alphasort);//scan a directory for matching entries
  	if(entries < 0) 
  	{ 
    		perror("scandir err");
		return -1;
  	}

	//assemble the html head
  	written = write_buffer(p, buf_left, "<html>%s<head>%s", CRLF, CRLF);
  	if(written < 0) 
	{
		perror("write_buffer err");
    		return -1;
  	}
  	p += written;
  	buf_left -= written;

  	written = write_buffer(p, buf_left, "<title>%s</title>%s</head>%s", basename(requester->path), CRLF, CRLF);
  	if(written < 0) 
	{
		perror("write_buffer err");
    		return -1;
  	}
  	p += written;
  	buf_left -= written;

  	written = write_buffer(p, buf_left, "<body>%s<h1>Directory Listing For %s</h1>%s<p>%s", 
					CRLF, basename(requester->path), CRLF, CRLF);
  	if(written < 0) 
	{
		perror("write_buffer err");
    		return -1;
  	}
  	p += written;
  	buf_left -= written;

	//iterate directory entry
  	for(i = 0; i < entries; i++) 
	{
    		if(buf_left < strlen(namelist[i]->d_name) + 2) 
		{ 
			//if the buffer is full enough, it should write out first
      			if(write(socket, buf, sizeof(buf) - buf_left) < 0) 
			{
        			perror("error writing directory listing");
        			return -1;
      			}
      			p = buf;
      			buf_left = sizeof(buf);
    		}

		//each entry for each line expect for '.'
    		if((strlen(namelist[i]->d_name) >= 1) && (namelist[i]->d_name[0] != '.')) 
		{
      			written = write_buffer(p, buf_left, "%s%s", namelist[i]->d_name, CRLF);
      			if(written < 0) 
			{
				perror("write_buffer err");
        			return -1;
      			}
      			p += written;
      			buf_left -= written;
    		}
    		free(namelist[i]);
  	}
  	free(namelist);

	//if the left buf space not enough for html end head, write out
  	if(buf_left < buf_left_max) 
	{
    		if(write(socket, buf, sizeof(buf) - buf_left) < 0) 
		{
      			perror("error writing directory listing");
      			return -1;
    		}
    		p = buf;
    		buf_left = sizeof(buf);
  	}

  	written = write_buffer(p, buf_left, "</p>%s</body>%s</html>%s", CRLF, CRLF, CRLF);
  	if(written < 0) 
	{
		perror("write_buffer err");
    		return -1;
  	}
  	p += written;
  	buf_left -= written;
	
	//write the remining buf stuffs out to the socket
  	if (write(socket, buf, sizeof(buf) - buf_left) < 0) 
	{
    		perror("error writing directory listing");
    		return -1;
  	}

  	return 0;
}

/*
	FUNC DESCRIPTION:
		This function is used to execute the cgi and set the request parameter
		into the env
	PARAMETER:
		struct request * requester:
			request info struct
		struct flags * flag:
			using the flag's dir
		int * uri_status:
			the status used to send to client
		char * cgi_path:
			the realpath of request uri for cgi
		int socket:
			the client socket
	RETURN VALUE:
		< 0 something err
		= 0 success
*/
int execute_cgi(struct request * requester, struct flags * flag, int * uri_status, char * cgi_path, 
				int socket, struct response * responser)
{
	char query_env[query_length];	
	bzero(query_env, sizeof(query_env));
	char meth_env[method_length];
	bzero(meth_env, sizeof(meth_env));
	char length_env[content_length_cgi];
	bzero(length_env, sizeof(length_env));
	char type_env[content_type_length];
	bzero(type_env, sizeof(type_env));

	if(requester->method == REQUEST_METHOD_GET || requester->method == REQUEST_METHOD_HEAD)
	{
		if(requester->query_string != NULL)				
		{
			(void)sprintf(query_env, "QUERY_STRING=%s", requester->query_string);
		}
		if(requester->method == REQUEST_METHOD_GET)
		{
			(void)sprintf(meth_env, "REQUEST_METHOD=GET");
		}
		if(requester->method == REQUEST_METHOD_HEAD)
		{
			(void)sprintf(meth_env, "REQUEST_METHOD=HEAD");
		}
	}
	else
	{
		*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
		return -1;		
	}
	//put into the length and type
<<<<<<< HEAD
	sprintf(length_env, "CONTENT_LENGTH=%d", responser->content_length);
	sprintf(type_env, "CONTENT_TYPE=%s", responser->content_type);
	
	putenv(query_env);
	putenv(meth_env);
	putenv(length_env);
	putenv(type_env);
	
	dup2(socket, STDOUT_FILENO);
		
	execlp(cgi_path, cgi_path, (char *)NULL);
	*uri_status = RESPONSE_STATUS_OK;
	return 0;
=======
	(void)sprintf(length_env, "CONTENT_LENGTH=%d", requester->content_length);
	(void)sprintf(type_env, "CONTENT_TYPE=%s", requester->content_type);
	
	if(pipe(cgi_pipe) < 0)
	{
		*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
		return -1;	
	}
	if((pid = fork()) < 0)
	{
		*uri_status = RESPONSE_STATUS_INTERNAL_SERVER_ERROR;
		return -1;
	}
	if(pid == 0)
	{
		//child
		/* write to the pipe after execl */
		if(dup2(cgi_pipe[1], STDOUT_FILENO) == -1){
			perror("Duplicate fail");
		}
		
		if(close(cgi_pipe[0]) == -1){
			perror("Cannot close the file");
		}

		//store into the env
		putenv(query_env);
		putenv(meth_env);
		putenv(length_env);
		putenv(type_env);
		
		if(execl(cgi_path, cgi_path, (char *) NULL) == -1){
			perror("Execute CGI fail");
		}
		exit(0);
	}
	else
	{
		//parent
		if((close(cgi_pipe[1])) == -1){
			perror("Cannot close the file");	
		}
		while(read(cgi_pipe[0], &c, 1) > 0)
		{
			send(socket, &c, 1, 0);
		}
		if((close(cgi_pipe[0])) == -1){
			perror("Cannot close pipe");
		}

		if(waitpid(pid, &status, 0) == -1){
			perror("Cannot read the data");
		}
		*uri_status = RESPONSE_STATUS_OK;
		return 0;
	}
>>>>>>> d9e1aecf356fee9cc480c282832da9e4b2d622a6
}

/*
	FUNC DESCRIPTION:
		This func is used to write the log information into the 
		flag->lflag file and according to the specific format.
	PARAMETER:
		int fd:
			write to..
		struct logging * log:
			the log information struct
	RETURN VALUE:
		< 0 fail
		= 0 success
*/
int write_to_log(int fd, struct logging * log)
{
	char buf[BUFSIZE];
	int res = 0;

	(void)snprintf(buf, sizeof(buf), "%s %s \"%s\" %s %s\n", 
	log->remoteip, log->request_time, log->request_lineq, log->request_status, log->response_size);
	
	if((res = write(fd, buf, strlen(buf))) < 0)
	{
		perror("Write error");
		return -1;
	}
	return 0;
}
