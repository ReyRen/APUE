#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/shm.h>
/*
这个是消费者：
	将创建一个共享内存段，然后把写到它里面的数据都显示出来。
*/
#include "shm_com.h"
int main()
{
	int running = 1;
  	void *shared_memory = (void *)0;//这样定义是为了防止当右值，因为void本身就是一个不合法的数据类型
  	struct shared_use_st *shared_stuff;
  	int shmid;

  	srand((unsigned int)getpid());
	/*
		key : 和信号量一样，程序需要提供一个参数key,
		它有效地为共享内存段命名
		有一个特殊的键值IPC_PRIVATE
		它用于创建一个只属于创建进程的共享内存
		通常不会用到
		size: 以字节为单位指定需要共享的内存容量。
		shmflag: 包含9个比特的权限标志，
			它们的作用与创建文件时使用的mode标志是一样。
			由IPC_CREAT定义的一个特殊比特必须和权限标志按位或
			才能创建一个新的共享内存段。
		
	*/
  	shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);

  	if (shmid == -1) {
    		fprintf(stderr, "shmget failed\n");
    		exit(EXIT_FAILURE);
  	}
	/*
		第一次创建共享内存段时，它不能被任何进程访问。
		要想启动对该内存的访问，
		必须将其连接到一个进程的地址空间
		这个工作由shmat函数完成
		
		shm_id : 由shmget返回的共享内存标识。
		shm_add: 指定共享内存连接到当前进程中的地址位置。
			它通常是一个空指针， 
			表示让系统来选择共享内存出现的地址。
		shmflg : 是一组标志。
			它的两个可能取值是:
			SHM_RND, 和shm_add联合使用，
			用来控制共享内存连接的地址。
			SHM_RDONLY, 它使连接的内存只读	
	*/
  	shared_memory = shmat(shmid, (void *)0, 0);//让程序可以访问这个共享内存

  	if (shared_memory == (void *)-1) {
    		fprintf(stderr, "shmat failed\n");
    		exit(EXIT_FAILURE);
  	}
	//将shared_memory分配给shared_stuff,然后它输出written_by_you中的文本。
	//循环将一直执行到在written_by_you中找到end字符串为止
  	printf("Memory attached at %X\n", (int)shared_memory);
	shared_stuff = (struct shared_use_st *)shared_memory;//这里就是指定了这个数据结构就是在这个进程已经映射了的共享内存中
  	shared_stuff->written_by_you = 0;

  	while(running) 
  	{
    		if (shared_stuff->written_by_you) 
    		{
      			printf("You wrote: %s", shared_stuff->some_text);

      			sleep( rand() % 4 ); /* make the other process wait for us ! */
      			shared_stuff->written_by_you = 0;

      			if (strncmp(shared_stuff->some_text, “end”, 3) == 0) {
       			running = 0;
      			}
    		}
  	}
//最后，共享内存被分离，然后被删除:
	if (shmdt(shared_memory) == -1) 
  	{
    		fprintf(stderr, "shmdt failed\n");
    		exit(EXIT_FAILURE);
  	}
	/*
		shm_id : 是shmget返回的共享内存标识符。
		command: 是要采取的动作
			它可以取3个值:
			IPC_STAT  把shmid_ds结构中的数据设置为共享内存的当前关联值
			IPC_SET   如果进程有足够的权限
				就把共享内存的当前关联值设置为shmid_ds结构中给出的值
			IPC_RMID  删除共享内存段
			buf    : 是一个指针，
				包含共享内存模式和访问权限的结构。
	*/
  	if (shmctl(shmid, IPC_RMID, 0) == -1) 
  	{
    		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
    		exit(EXIT_FAILURE);
  	}

  	exit(EXIT_SUCCESS);
}
