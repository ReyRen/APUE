#include <unistd.h>
#include "util.h"

int
main(int argc, char * argv[])
{
	int command;
	int res;
	char real_path[MAX_BUF];
	struct flags flag;

	flags_init(&flag);
	while((command = getopt(argc, argv, PARAMETER_FORMAT)) != -1)
	{
		switch(command)
		{
			case 'l':
				flag.l_flag = 1;
				flag.l_filter = optarg;
				break;
			case 'r':
				flag.r_flag = 1;	
				flag.r_filter = optarg;
				break;
			case 'v':
				flag.v_flag = 1;
				strncpy(flag.v_volumeID, optarg, strlen(optarg));
				break;
			case 'h':
				usage();
				exit(SUCCESS);
				break;
			default:
				usage();
				exit(FAILED);
		}
	}
	argc -= optind;
	argv += optind;
	if (argc != 1)
	{
		usage();
		exit(FAILED);
	}
	memset(real_path, 0, sizeof(real_path))	;
	res = get_real_path(argv[0], real_path);
	if(res != SUCCESS)
	{
		fprintf(stderr, "get_real_path() err\n");
		return res;
	}
	flag.backup_dir = real_path;
	res = handle_helper(&flag);
	if(res != SUCCESS)
	{
		fprintf(stderr, "handle_helper() err\n");
		return res;
	}
	return res;
}

