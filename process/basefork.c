#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//fork
int main(void)
{
	pid_t pid;
	int num = 10;

	printf("Before fork :%d\n", getpid());
	pid = fork();
	if(pid == -1)
	{
		perror("fork error");
		return 0;
	}
	if(pid > 0)
	{
		printf("parent :%d\n", getpid());
		num++;
		printf("parent num: %d\n", num);
	}
	else
	{
		printf("child :%d\n", getpid());
		num++;
                printf("child num: %d\n", num);
	}
	printf("After fork :%d\n", getpid());
	return 0;
}
//系统的错误码，放到了全局变量errno里面
//在程序中，perror可以把errno对应的string打印出来
//如果在shell中，可以通过perror 错误码  打印出字符串

/*
fork函数特点：
	1.一次调用两次返回

fork内核中执行过程:
	1.一执行fork这个system call 函数，就陷入了内核，
	2.马上内核创建了两个进程，每个进程在他各自的内存空间返回到用户内存空间，现在是两个进城，所以返回两次
fork返回值为什么设计成pid>0是父进程分支，=0是子进程分支？
	1.因为一个进程可以创建很多子进程，那么每生一个孩子，就把孩子的pid给了父亲，这样父进程容易管理孩子，还有事父找孩子比较难
宏观上讲：
	两个分支在同时向下执行代码
微观上讲：
	每个进程单独分CPU段
为什么子进程是从fork之后运行而不是从main开始运行呢？
	fork创建子进程的机制是写时复制：
		子进程copy父进程的代码段，堆栈段，数据段，PCB进程控制块（就是内核管理进程的数据结构,既然已经拷贝了父的场景，就没必要再从头运行一遍了）

关于copy问题的深入探讨:
	只有当子进程修改这个变量的一瞬间内核才会做copy工作，这就是写时copy，就是只有往变量里面写的时候，说明了子进程和父进程的变量发生变化的时候
	执行真正的copy，但是这个copy只是把这个变量所在的内存的页拷贝了，因为内核里面是进行的段式管理和页式管理，从虚拟地址到物理地址的映射。一个进程
	有好多的页，加入子进程又要用第三页，但是在子进程中没有，内核就会发出一个缺页终端也就是软中断，相当于阻断了子进程，然后一页一页的拷贝过来再走
*/

