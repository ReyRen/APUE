#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE); \
	}while(0)

union semun
{
        int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO*/
};

int sem_create(key_t key)
{
	int semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL);
	if(semid == -1)
	{
		ERR_EXIT("semget");
	}
	return semid;
}

int sem_open(key_t key)
{
	int semid = semget(key, 0 ,0);
	if(semid == -1)
	{
		ERR_EXIT("semget");
	}
	return semid;
}

int sem_p(int semid)
{
	struct sembuf sb = {0, -1, 0};
	int ret = semop(semid, &sb, 1);
	if(ret == -1)
	{
		ERR_EXIT("semop");
	}
	return ret;
}

int sem_v(int semid)
{
	struct sembuf sb = {0, 1, SEM_UNDO};/*SEM_UNDO 进程终止时，进程做的P操作和v操作会撤销回到原点*/
	/*也就类似，执行了p操作，但是进程结束都没执行v操作，就属于是意外进程终止，所以会直接复原*/
/*	struct sembuf sb = {0, 1, 0}; */
	int ret = semop(semid, &sb, 1);
	if(ret == -1)
	{
		ERR_EXIT("semop");
	}
	return ret;
}

int sem_d(int semid)
{
	int ret = semctl(semid, 0, IPC_RMID, 0);
	if(ret == -1)
	{
		ERR_EXIT("semctl");
	}
	return ret;
}

int sem_setval(int semid, int val)
{
	union semun su;
	su.val = val;
	int ret = semctl(semid, 0, SETVAL, su);
	if(ret == -1)
	{
		ERR_EXIT("semctl");
	}
	printf("value updated...\n");
	return ret;
}	

int sem_getval(int semid)
{
	int ret = semctl(semid, 0, GETVAL, 0);
	if(ret == -1)
	{
		ERR_EXIT("semctl");
	}
	printf("current val is %d\n", ret);
	return ret;
}

int sem_getmode(int semid)
{
	union semun su;
	struct semid_ds sem;
	su.buf = &sem;
	int ret = semctl(semid, 0, IPC_STAT, su);
	if(ret == -1)
	{
		ERR_EXIT("semctl");
	}
	printf("current permission is %o\n", su.buf->sem_perm.mode);
	return ret;
}

int sem_setmode(int semid, char * mode)
{
	union semun su;
	struct semid_ds sem;
	su.buf = &sem;
	int ret = semctl(semid, 0, IPC_STAT, su);
	if(ret == -1)
	{
		ERR_EXIT("semctl");
	}
	printf("current permission is %o\n", su.buf->sem_perm.mode);
	sscanf(mode, "%o", (unsigned int *)&su.buf->sem_perm.mode);
	ret = semctl(semid, 0, IPC_SET, su);
	if(ret == -1)
	{
		ERR_EXIT("semctl");
	}
	printf("permission updated...\n");
	return ret;
}

void usage(void)
{
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "semtool -c 创建信号量\n");
	fprintf(stderr, "semtool -d 删除信号量\n");
	fprintf(stderr, "semtool -p 信号量p操作\n");
	fprintf(stderr, "semtool -v 信号量v操作\n");
	fprintf(stderr, "semtool -s <val> 向信号量设置初始值\n");
	fprintf(stderr, "semtool -g 获取信号量的值\n");
	fprintf(stderr, "semtool -f 打开权限\n");
	fprintf(stderr, "semtool -m <mode> 设置权限\n");
}

int main(int argc, char * argv[])
{
	int opt;
	opt = getopt(argc, argv, "cdvs:gfm:");
	if(opt == '?')
	{
		exit(EXIT_FAILURE);
	}
	if(opt == -1)
	{
		usage();
		exit(EXIT_FAILURE);
	}
	//第一个参数是一个存在的路径
	//第二个参数非0
	key_t key = ftok(".", 's');
	int semid;
	switch(opt)
	{
		case 'c':
			sem_create(key);
			break;
		case 'p':
			semid = sem_open(key);
			sem_p(semid);
			sem_getval(semid);
			break;
		case 'v':
			semid = sem_open(key);
			sem_v(semid);
			sem_getval(semid);
			break;
		case 'd':
			semid = sem_open(key);
			sem_d(semid);
			break;
		case 's':
			semid = sem_open(key);
			sem_setval(semid, atoi(optarg));
			break;
		case 'g':
			semid = sem_open(key);
			sem_getval(semid);
			break;
		case 'f':
			semid = sem_open(key);
			sem_getmode(semid);
			break;
		case 'n':
			semid = sem_open(key);
			sem_setmode(semid, argv[2]);
			break;
	}
	return 0;
}
