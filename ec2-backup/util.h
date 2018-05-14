#ifndef EC2_BACKUP_UTIL
#define EC2_BACKUP_UTIL
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#define PARAMETER_FORMAT "hl:r:v:"

#define MAX_BUF 4096
#define SUCCESS 0
#define FAILED 1

/*sleep time to ensure the instance OK*/
#define CREATE_INSTANCE_SEC 10
#define QUERY_IP_SEC 10
#define GET_REGION_SEC 10
#define GET_AVALIABLE_ZONE_SEC 5
#define DETACH_SEC 5
#define CREATE_VOLUME 2
#define ATTACH_VOLUME 2
#define COMMAND_EXECUTE 70

#define _AWS_FLAG_ "EC2_BACKUP_FLAGS_AWS"
#define _AWS_SSH_ "EC2_BACKUP_FLAGS_SSH"
#define _AWS_VERBOSE_ "EC2_BACKUP_VERBOSE"

#define _AMI_ID_DEFAULT_ "ami-569ed93c"
#define _SECURITY_GROUP_DEFAULT_ "cs631-security-group"
#define _KEY_NAME_DEFAULT_ "cs615-key-pair"
#define _COUNT_DEFAULT_ "1"
#define _INSTANCE_TYPE_DEFAULT_ "t1.micro"
#define _QUERY_METHOD_DEFAULT_ "'Instances[0].InstanceId'"

struct flags
{
	int l_flag;
	int r_flag;
	int v_flag;

	int dir_size;
	char* l_filter;
	char* r_filter;
	char v_volumeID[MAX_BUF];
	char* backup_dir;

/*ec2 parameter*/
	char public_ip[MAX_BUF];
	char instance_id[MAX_BUF];
	char region[MAX_BUF];
	char avaliable_zone[MAX_BUF];
/*comm*/
	char l_comm[MAX_BUF];
	char r_comm[MAX_BUF];
};

int
handle_helper(struct flags * flag);

void 
flags_init(struct flags * flag);

void
usage();

int
get_ssh_ip(struct flags* flag);


int
get_sshKey_name(char * ssh_flag, 
		int num);

int
handle_flags_aws(char * ssh_flag_result, 
		int num);

int
get_verbose();

int
check_file_permission(char * filename);

int
get_system_result(char * buf_system_result, 
		int num, 
		char * system_command,
		int sleep_time);

int
create_volume(struct flags* flag);

int
get_real_path(char * pathname, 
		char * real_path);

int
handle_volume_size(struct flags* flag);

int
attach_volume(struct flags* flag);

int
local_filter(struct flags* flag);

int
remote_filter(struct flags* flag);

int
comm(struct flags* flag);

int
terminate_instance(struct flags* flag);

int
detach_volume(struct flags* flag);

int 
ssh_command(char * system_command,int sleep_time);

#endif
