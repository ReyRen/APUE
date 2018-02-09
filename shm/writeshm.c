#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

typedef struct _Teacher
{
	char name[64];
	int age;
} Teacher;


//
/*
int shmget(key_t key, size_t size, int shmflg);
//

PSIS
       #include <sys/ipc.h> #include <sys/shm.h>

       int shmctl(int shmid, int cmd, struct shmid_ds *buf);

DESCRIPTION
       shmctl()  performs  the  control  operation specified by cmd on the shared memory segment whose identifier is
       given in shmid.

       The buf argument is a pointer to a shmid_ds structure, defined in <sys/shm.h> as follows:

           struct shmid_ds {
               struct ipc_perm shm_perm;    // Ownership and permissions //
               size_t          shm_segsz;   // Size of segment (bytes) //
               time_t          shm_atime;   // Last attach time //
               time_t          shm_dtime;   // Last detach time //
               time_t          shm_ctime;   // Last change time //
               pid_t           shm_cpid;    // PID of creator //
               pid_t           shm_lpid;    // PID of last shmat()/shmdt() //
               shmatt_t        shm_nattch;  // No. of current attaches //
               ...
           };

       The ipc_perm structure is defined in <sys/ipc.h> as  follows  (the  highlighted  fields  are  settable  using
       IPC_SET):

           struct ipc_perm {
               key_t key;            // Key supplied to shmget() //
               uid_t uid;            // Effective UID of owner //
               gid_t gid;            // Effective GID of owner //
               uid_t cuid;           // Effective UID of creator //
               gid_t cgid;           // Effective GID of creator //
               unsigned short mode;  // Permissions + SHM_DEST and
                                        SHM_LOCKED flags //
               unsigned short seq;   // Sequence number //
           };
*/

//
//key        shmid      owner      perms      bytes      nattch     status      
//0x00002234 131073     wbm01     666        68         0    
//  
int main(int argc, char *argv[])
{
	int ret = 0;
	int 	shmid;
	//相当于打开文件，文件不存
	shmid = shmget(0x2234, sizeof(Teacher), IPC_CREAT | 0666); 
	if (shmid == -1)
	{
		perror("shmget err");
		return errno;
	}
	printf("shmid:%d \n", shmid);
	Teacher *p = NULL;

	p = shmat(shmid, NULL, 0);//相当于p是共享内存的首地址
	if (p == (void *)-1 )
	{
		perror("shmget err");
		return errno;
	}
	
	strcpy(p->name, "aaaa");
	p->age = 33;
//shmdt()  detaches  the  shared  memory segment located at the address specified by shmaddr from the address space of the calling process.	
	shmdt(p);
		
	printf("键入1 删除共享内存，其他不删除\n");
	
	int num; 
	scanf("%d", &num);
	if (num == 1)
	{
		ret = shmctl(shmid, IPC_RMID, NULL);
		if (ret < 0)
		{
			perror("rm errr\n");
		}
	}                 

	return 0;	
}
/*
	如果共享内存被别的程序占用，删除共享内存，则不会立马删除
	但是会出现一个现象，key编程了0，也就是变成私有的了
*/
