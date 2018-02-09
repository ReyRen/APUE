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

union semun 
{
	int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO*/
};

int sem_create(key_t key)
{
	int semid = 0;
	/*int semget(key_t key, int nsems, int semflg);*/
	semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL);
	if(semid == -1)
	{
		perror("segmet");
		if(errno == EEXIST)
		{
			if(errno == EEXIST)
			{
				printf("检测到信号量集已经存在\n");
				return -1;		
			}
		}
	}
	return semid;
}

int sem_open(key_t key)
{
	int semid = 0;
	semid = semget(key, 1, 0666);
	if(semid == -1)
	{
		perror("semget");
		return -1;
	}
	return semid;
}

int sem_setval(int semid, int value)
{
	/*int semctl(int semid, int semnum, int cmd, ...);*/
	/*
		semnum:信号集中信号量的序号
		cmd:将要采取的动作（有三个可取值）
	*/	
	/*
		union semun {
               int              val;     Value for SETVAL 
               struct semid_ds *buf;     Buffer for IPC_STAT, IPC_SET 
               unsigned short  *array;   Array for GETALL, SETALL 
               struct seminfo  *__buf;   Buffer for IPC_INFO
                                           (Linux-specific) 
           };
	*/
	int ret = 0;
	union semun su;
	su.val = value;
	ret = semctl(semid, 0, SETVAL, su);
	return ret;
	
}

int sem_getval(int semid)
{
	int ret = 0;
	int val;
	union semun su;
	ret = semctl(semid, 0, GETVAL, su);
	val = su.val;
	printf("val:%d\n", val);
	return ret;
}

int sem_p(int semid)
{
	int ret = 0;
	/*unsigned short sem_num;   semaphore number */
	struct sembuf buf = {0, -1, 0};/*参数3为0就是阻塞情况*/
	ret = semop(semid, &buf, 1);/*相当于用一个结构体的形式告诉内核进行p操作*/
	return ret;
}
int sem_v(int semid)
{
	int ret = 0;
	struct sembuf buf = {0, 1, 0};
	ret = semop(semid, &buf, 1);
	return ret;
}
int main(void)
{
	int semid;
	semid = sem_open(0x1111);
	sem_setval(semid, 1);
	sem_getval(semid);

	sem_p(semid);
	printf("dddddddddddddd\n");
	sem_v(semid);
	return 0;
}
