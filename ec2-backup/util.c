#include "util.h"
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <libgen.h>

void
usage()
{
	printf("usage: ec2-backup [-h] [-l filter] \
[-r filter] [-v volume-id] dir\n");
}

int
handle_helper(struct flags * flag)
{	
	char * aws_ssh;
	char SSH_KEY[MAX_BUF];
	int res;
	
	memset(SSH_KEY, 0, sizeof(SSH_KEY));

/*Get the key pair name and check the permission*/
	aws_ssh = getenv(_AWS_SSH_);
	if(aws_ssh != NULL)
	{
		res = get_sshKey_name(SSH_KEY, sizeof(SSH_KEY));
		if(res != SUCCESS)
		{
			fprintf(stderr, "get_sshKey_name() err\n");
			return res;
		}
		res = check_file_permission(SSH_KEY);
		if(res != SUCCESS)
		{
			fprintf(stderr, "check_file_permission() err\n");
			return res;
		}
	}
	
/*handle ssh to instance to get the public IP*/
	res = get_ssh_ip(flag);
	if(res != SUCCESS)
	{
		fprintf(stderr, "get_ssh_ip() err\n");
		return res;
	}
/*If the -v not specified, should create the volume*/
	if(flag->v_flag == 0)
	{
		res = create_volume(flag);
		if(res != SUCCESS)
		{
			fprintf(stderr, "create_volume() err\n");
			return res;
		}
	}
/*Attach the volume to the system*/
	res = attach_volume(flag);
	if(res != SUCCESS)
	{
		fprintf(stderr, "attach_volume() err\n");
		return res;
	}
/*-l filter and -r filter*/
	if(flag->l_flag == 1)
	{
		res = local_filter(flag);
		if(res != SUCCESS)
		{
			fprintf(stderr, "local_filter() err\n");
			return res;
		}
	}
	if(flag->r_flag == 1)
	{
		res = remote_filter(flag);
		if(res != SUCCESS)
		{
			fprintf(stderr, "remote_filter() err\n");
			return res;
		}
	}
	res = comm(flag);
	if(res != SUCCESS)
	{
		fprintf(stderr, "comm() err\n");
		return res;
	}
/*detach and terminate*/

	res = detach_volume(flag);
	if(res != SUCCESS)
	{
		fprintf(stderr, "detach_volume() err\n");
		return res;
	}
	res = terminate_instance(flag);
	if(res != SUCCESS)
	{
		fprintf(stderr, "terminate_instance() err\n");
		return res;
	}
	printf("%s\n", flag->v_volumeID);
	
	return SUCCESS;
}

int
local_filter(struct flags* flag)
{
	sprintf(flag->l_comm, "%s | ", flag->l_filter);

	return SUCCESS;
}

int
remote_filter(struct flags* flag)
{
	sprintf(flag->r_comm, "\"%s | dd of=/dev/xbd2\"", flag->r_filter);	

	return SUCCESS;
}

int
comm(struct flags* flag)
{
	char comm_result[MAX_BUF];
	char command_buf[MAX_BUF];
	int res;

	memset(comm_result, 0, sizeof(comm_result));
	memset(command_buf, 0, sizeof(command_buf));

	if(getenv(_AWS_SSH_) == NULL)
	{
	sprintf(command_buf, "tar cf - -P  %s | %s ssh -o StrictHostKeyChecking=no \
 root@%s %s", 
		flag->backup_dir, flag->l_comm,  
	        flag->public_ip, flag->r_comm);		
	}
	else
	{
		sprintf(command_buf, "tar cf - -P %s | %s ssh -o StrictHostKeyChecking=no \
%s root@%s %s", 
		flag->backup_dir, flag->l_comm, getenv(_AWS_SSH_), 
		flag->public_ip, flag->r_comm);
	}
	if(get_verbose() == 0)
	{
		printf("Backing up...\n");
	}

	res = ssh_command(command_buf,COMMAND_EXECUTE);
	if(res != SUCCESS)
	{
		fprintf(stderr, "get_system_result() err\n");
		return res;
	}
	if(get_verbose() == 0)
	{
		printf("Back up complete...\n");
	}
	return SUCCESS;
	
}

int
attach_volume(struct flags* flag)
{
	char attach_volume_result[MAX_BUF];
	char command_result[MAX_BUF];
	int res;

	memset(attach_volume_result, 0, sizeof(attach_volume_result));
	memset(command_result, 0, sizeof(command_result));
	sprintf(attach_volume_result, "aws ec2 attach-volume --volume-id\
 %s --instance-id %s --device /dev/sdf", flag->v_volumeID, flag->instance_id);
	if(get_verbose() == 0)
	{
		printf("Attaching the volume...\n");
	}
	res = get_system_result(command_result, 
				sizeof(command_result),
				attach_volume_result,
				ATTACH_VOLUME);
	if(res != SUCCESS)
	{
		fprintf(stderr, "get_system_result() err\n");
		return res;
	}
	if(get_verbose() == 0)
	{
		printf("Attach the volume complete...\n");
	}
	return SUCCESS;
}

int
get_ssh_ip(struct flags* flag)
{
	int res;
	char ssh_flag_result[MAX_BUF];
	char buf_instanceID_result[MAX_BUF];
	char buf_publicIP_result[MAX_BUF];
	char buf_getIP[MAX_BUF];
	
	memset(ssh_flag_result, 0, sizeof(ssh_flag_result));
	memset(buf_instanceID_result, 0, sizeof(buf_instanceID_result));
	memset(buf_publicIP_result, 0, sizeof(buf_publicIP_result));
	//memset(buf_getIP, 0, 4096);

	/*get the instance ID*/
	res = handle_flags_aws(ssh_flag_result, sizeof(ssh_flag_result));
	if(res != SUCCESS)
	{
		fprintf(stderr, "handle_flags_aws() err\n");
		return res;
	}
	if(get_verbose() == 0)
	{
		printf("Establishing the instance....\n");
	}
	res = get_system_result(buf_instanceID_result, 
			sizeof(buf_instanceID_result), 
			ssh_flag_result,
			CREATE_INSTANCE_SEC);
	if(res != SUCCESS)
	{
		fprintf(stderr, "get_system_result() err\n");
		return res;
	}
	if(get_verbose() == 0)
	{
		printf("Establish instance complete...\n");
	}
	strncpy(flag->instance_id, buf_instanceID_result, 
		strlen(buf_instanceID_result));

	/*get the public IP address according the buf_instanceID_result*/
	char str_0[] = "'Reservations[0].Instances[0].PublicDnsName'";
	sprintf(buf_getIP, "aws ec2 describe-instances --query %s --instance-ids %s", 
		str_0, flag->instance_id);
	if(get_verbose() == 0)
	{
		printf("Getting the public IP address...\n");
	}
	res = get_system_result(buf_publicIP_result, 
			sizeof(buf_publicIP_result), 
			buf_getIP,
			QUERY_IP_SEC);
	if(res != SUCCESS)
	{
		fprintf(stderr, "get_system_result() err\n");
		return res;
	}
	if(get_verbose() == 0)
	{
		printf("Get the public IP address complete...\n");
	}
	strncpy(flag->public_ip, buf_publicIP_result, strlen(buf_publicIP_result));

	return SUCCESS;
}

int 
handle_endofstring(char* string, int num)
{
	int i;
	if(string == NULL || num == 0)	
	{
		return FAILED;
	}
	for(i = 0; i < num; i++)
	{
		if(string[i] == '\n')
		{
			string[i] = '\0';
			return SUCCESS;
		}
	}	
	return SUCCESS;
}

int
create_volume(struct flags* flag)
{
	int res;
	char region[MAX_BUF];
	char avaliable_zone[MAX_BUF];
	char avaliable_zone_command[MAX_BUF];
	char create_volume_coomand[MAX_BUF];
	char create_volume_result[MAX_BUF];

	memset(region, 0, sizeof(region));
	memset(avaliable_zone, 0, sizeof(avaliable_zone));
	memset(avaliable_zone_command, 0, sizeof(avaliable_zone_command));
	memset(create_volume_coomand, 0, sizeof(create_volume_coomand));
	memset(create_volume_result, 0, sizeof(create_volume_result));
	if(get_verbose() == 0)
	{
		printf("Getting the region...\n");
	}
	res = get_system_result(region, sizeof(region), 
			"aws configure get region",
			GET_REGION_SEC);
	if(res != SUCCESS)
	{
		fprintf(stderr, "get_system_result() err\n");
		return res;
	}
	if(get_verbose() == 0)
	{
		printf("Get the region complete...\n");
	}
	strncpy(flag->region, region, strlen(region));

	sprintf(avaliable_zone_command, "aws ec2 describe-instances \
--instance-ids %s --output text --query \
'Reservations[*].Instances[*].[Placement.AvailabilityZone]'", 
		flag->instance_id);
	if(get_verbose() == 0)
	{
		printf("Getting the avaliable zone...\n");
	}
	res = get_system_result(avaliable_zone, 
			sizeof(avaliable_zone), 
			avaliable_zone_command,
			GET_AVALIABLE_ZONE_SEC);
	if(res != SUCCESS)
	{
		fprintf(stderr, "get_system_result() err\n");
		return res;
	}
	if(get_verbose() == 0)
	{
		printf("Get the avaliable zone complete...\n");
	}
	strncpy(flag->avaliable_zone, avaliable_zone, 
		strlen(avaliable_zone));
	res = handle_volume_size(flag);
	if(res != SUCCESS)
	{
		fprintf(stderr, "handle_volume_size() err\n");
		return res;
	}
	sprintf(create_volume_coomand, "aws ec2 create-volume \
--size %d --region %s --availability-zone %s \
--volume-type gp2 --output text | awk '{print $7}'", 
		flag->dir_size, region, avaliable_zone);
	if(get_verbose() == 0)
	{
		printf("Creating the volume...\n");
	}
	res =get_system_result(create_volume_result, 
			sizeof(create_volume_result), 
			create_volume_coomand,
			CREATE_VOLUME);
	if(res != SUCCESS)
	{
		fprintf(stderr, "get_system_result() err\n");
		return res;
	}
	if(get_verbose() == 0)
	{
		printf("Create the volume complete...\n");
	}
	strncpy(flag->v_volumeID, create_volume_result, 
			strlen(create_volume_result));
	//printf("%s", create_volume_result);

	return SUCCESS;
}

int
get_system_result(char * buf_system_result, 
		int num, 
		char * system_command,
		int sleep_time)
{
	int fd[2];
	char buf[MAX_BUF];
	int backfd;
	int res;

	memset(buf, 0, sizeof(buf));
	if(buf_system_result == NULL || num == 0)
	{
		return FAILED;
	}
	if(pipe(fd) != 0)
	{
		fprintf(stderr, "pipe err\n");
		return FAILED;
	}
	backfd = dup(STDOUT_FILENO);/*for backup fileNO*/
	dup2(fd[1],STDOUT_FILENO);
	system(system_command);
	sleep(sleep_time);
	read(fd[0], buf_system_result, num);
	res = handle_endofstring(buf_system_result, num);
	if(res != SUCCESS)
	{
		fprintf(stderr, "handle_endofstring() err\n");
		return FAILED;
	}
	dup2(backfd,STDOUT_FILENO);
	return res;
}

int
handle_flags_aws(char * ssh_flag_result, int num)
{
	char aws_flag[MAX_BUF];
	char delims[] = " ";  
	char * result = NULL;

	memset(aws_flag, 0, sizeof(aws_flag));

	if(ssh_flag_result == NULL || num == 0)
	{
		return FAILED;
	}

	char  flag_str_0_0[] = "--image-id";
        char  flag_str_0_1[PATH_MAX] = _AMI_ID_DEFAULT_;
        char  flag_str_1_0[] = "--security-groups";
        char  flag_str_1_1[PATH_MAX] = _SECURITY_GROUP_DEFAULT_;
        char  flag_str_2_0[] = "--key-name";
        char  flag_str_2_1[PATH_MAX] = _KEY_NAME_DEFAULT_;
        char  flag_str_3_0[] = "--count";
        char  flag_str_3_1[PATH_MAX] = _COUNT_DEFAULT_;
        char  flag_str_4_0[] = "--instance-type";
        char  flag_str_4_1[PATH_MAX] = _INSTANCE_TYPE_DEFAULT_;
        char  flag_str_5_0[] = "--query";
        char  flag_str_5_1[PATH_MAX] = _QUERY_METHOD_DEFAULT_;
	if(getenv(_AWS_FLAG_) != NULL)
	{
		strcpy(aws_flag, getenv(_AWS_FLAG_));
		result = strtok(aws_flag, delims); 
		
	while(result != NULL)
	{
		if(strcmp(result, flag_str_0_0) == 0)
		{
			strcpy(flag_str_0_1, strtok(NULL, delims));
		}
		else if(strcmp(result, flag_str_1_0) == 0)
		{
			strcpy(flag_str_1_1, strtok(NULL, delims));
		}
		else if(strcmp(result, flag_str_2_0) == 0)
		{
			strcpy(flag_str_2_1, strtok(NULL, delims));
		}
		else if(strcmp(result, flag_str_3_0) == 0)
		{
			strcpy(flag_str_3_1, strtok(NULL, delims));
		}
		else if(strcmp(result, flag_str_4_0) == 0)
		{
			strcpy(flag_str_4_1, strtok(NULL, delims));
		}
		else if(strcmp(result, flag_str_5_0) == 0)
		{
			strcpy(flag_str_5_1, strtok(NULL, delims));
		}
		result = strtok(NULL, delims);
	}
	}
	sprintf(ssh_flag_result, "aws ec2 run-instances %s %s %s %s %s %s %s %s %s %s %s %s", 
		flag_str_0_0, flag_str_0_1, flag_str_1_0, flag_str_1_1, flag_str_2_0,
		flag_str_2_1, flag_str_3_0, flag_str_3_1, flag_str_4_0, flag_str_4_1,
		flag_str_5_0, flag_str_5_1);
	return SUCCESS;
}

int
get_sshKey_name(char * ssh_flag, int num)
{
	if(ssh_flag == NULL || num == 0)
	{
		fprintf(stderr, "ssh_flag is NULL\n");
		return FAILED;
	}
	strncpy(ssh_flag, getenv(_AWS_SSH_)+3, num);
	
	return SUCCESS;
}

int
get_verbose()
{
	if(getenv(_AWS_VERBOSE_) != NULL)
	{
		return 0;
	}
	return 1;
}

int
check_file_permission(char * filename)
{
	char real_path[MAX_BUF];
	int res;

	memset(real_path, 0, sizeof(real_path));
	if(filename == NULL)
	{
		fprintf(stderr, "filename not found\n");
		return FAILED;
	}	
	res = get_real_path(filename, real_path);
	if(res == FAILED)
	{
		fprintf(stderr, "get_real_path() err\n");
		return res;
	}
	if(access(real_path, R_OK) != 0)
	{
		fprintf(stderr, "Key file permission deny\n");
		return FAILED;
	}
	return SUCCESS;
}

int 
get_real_path(char * pathname, char * real_path)
{
	char tmp[MAX_BUF];

	memset(tmp, 0, sizeof(tmp));

	if(pathname == NULL || real_path == NULL)
	{
		fprintf(stderr, "pathname is NULL in get_real_path()\n");
		return FAILED;
	}
	if(pathname[0] == '~')
	{
		pathname = pathname + 1;
		sprintf(tmp, "%s%s", getpwuid(getuid())->pw_dir, pathname);
	}
	else
	{
		sprintf(tmp, "%s", pathname);
	}
	if(realpath(tmp, real_path) == NULL)
	{
		fprintf(stderr, "realpath() err\n");
		return FAILED;
	}
	return SUCCESS;
}

int
handle_volume_size(struct flags* flag)
{
	struct stat buf;
	int res;
	double size;
	double tmp;
	
	res = stat(flag->backup_dir, &buf);
	if(res != 0)
	{
		fprintf(stderr, "stat() err\n");
		return FAILED;
	}
	size = (double)buf.st_size;
/*Convert to Gigbytes*/
	if(size < 1024*1024*512)
	{
		tmp = 0.5;
	}
	else{
		while(size > 1024*1024*1024)
		{
			tmp += 1;
			size = size - 1024*1024*1024;
		}
		if((size - 1024*1024*512) <= 0)
		{
			tmp += 0.5;
		}
	}
	flag->dir_size = (int)(tmp*2);
	return SUCCESS;
}
void 
flags_init(struct flags * flag)
{
	flag->l_flag = 0;
	flag->r_flag = 0;
	flag->v_flag = 0;

	flag->dir_size = 0;
	flag->l_filter = NULL;
	flag->r_filter = NULL;
	memset(flag->v_volumeID, 0, sizeof(flag->v_volumeID));
	flag->backup_dir = NULL;

	memset(flag->public_ip, 0, sizeof(flag->public_ip));
	memset(flag->instance_id, 0, sizeof(flag->instance_id));
	memset(flag->region, 0, sizeof(flag->region));
	memset(flag->avaliable_zone, 0, sizeof(flag->avaliable_zone));

	memset(flag->l_comm, 0, sizeof(flag->l_comm));
	memset(flag->r_comm, 0, sizeof(flag->r_comm));
}

int
terminate_instance(struct flags* flag)
{
        int res;
        char buf_system_result[MAX_BUF] ;
        char system_command[MAX_BUF];
        memset(buf_system_result, 0, sizeof(buf_system_result));
	memset(system_command, 0, sizeof(system_command));
        sprintf(system_command, "aws ec2 terminate-instances --instance-ids %s",
        flag->instance_id);

	if(get_verbose() == 0)
	{
		printf("Terminating the instance ...\n");
	}
        res = get_system_result(buf_system_result,
                          sizeof(buf_system_result),
                          system_command,
                          0);
        if(res != SUCCESS)
        {
                fprintf(stderr,"terminate_instance() err\n ");
                return res;
        }
	if(get_verbose() == 0)
	{
		printf("Terminate the instance complete...\n");
	}
        return  SUCCESS;
}

int
detach_volume(struct flags* flag)
{
        char buf_system_result[MAX_BUF] ;
        char system_command[MAX_BUF];
	int res;
	
        memset(buf_system_result, 0, sizeof(buf_system_result));
	memset(system_command, 0, sizeof(system_command));
	if(get_verbose() == 0)
	{
		printf("Detaching volume...\n");
	}
        sprintf(system_command, "aws ec2 detach-volume --volume-id %s",
        flag->v_volumeID);
        res = get_system_result(buf_system_result,
                          sizeof(buf_system_result),
                          system_command,
                          DETACH_SEC);
        if(res != SUCCESS)
        {
                fprintf(stderr,"detach_volume() err\n ");
                return res;
        }
	if(get_verbose() == 0)
	{
		printf("detach complete...\n");
	}
        return  SUCCESS;
}

int 
ssh_command(char * system_command,int sleep_time)
{
	int backfd , backfd_err;
        int nullFd;
	int res;
	/*FILE *fp = NULL;*/
        nullFd = open("/dev/null",O_RDWR);
	if(nullFd == -1)
	{
		fprintf(stderr, "open() err\n");
		return nullFd;
	}
        backfd = dup(STDOUT_FILENO);/*for backup fileNO*/
	if(backfd == -1)
	{
		fprintf(stderr, "dup() err\n");
		return backfd;
	}
	backfd_err = dup(STDERR_FILENO);
	if(backfd_err == -1)
	{
		fprintf(stderr, "dup() err\n");
		return backfd_err;
	}
    	//fp = fdopen(backfd, "r");
        res = dup2(STDOUT_FILENO,nullFd);
	if(res == -1)
	{
		fprintf(stderr, "dup() err\n");
		return res;
	}
        res = dup2(STDERR_FILENO,nullFd);
	if(res == -1)
	{
		fprintf(stderr, "dup() err\n");
		return res;
	}
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

        sleep(sleep_time);
        system(system_command);
	fflush(stdout);
	fflush(stderr);
/*recover*/
        dup2(backfd,STDOUT_FILENO);
        dup2(backfd_err,STDERR_FILENO);
	
        return SUCCESS;
}
