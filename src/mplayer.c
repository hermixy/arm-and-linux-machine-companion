#include "mplayer.h"

#include <stdio.h>
#include <sys/types.h> // open
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>		//close  read write   sleep usleep
#include <string.h>		//strlen
#include <errno.h>
#include <dirent.h>
#include <sys/mman.h>	//mmap munmap
#include <stdlib.h>     //system

// 定义管道文件的路径
#define FIFO 		"/tmp/fifo"

int fifo_fd = 0;

// 初始化管道，并打开
int fifo_init()
{
	if(access(FIFO, F_OK))  //返回值，存在为0，不存在为-1
	{
		if(mkfifo(FIFO, 0777)) //返回值，成功0，失败-1
		{
			perror("fifo init error\n");
			return -1;
		}
	}
	
	fifo_fd = open(FIFO, O_RDWR); // 打开管道文件
}

// 写数据到管道
int write_fifo(char *ctrl)
{
	write(fifo_fd, ctrl, strlen(ctrl));
}

void mplayer_start(char *video_path) // 播放视频
{
	char path[200] = {0}; // 定义了一个字符串数组
	
	sprintf(path, "mplayer -slave -quiet -input file=/tmp/fifo -geometry 0:0 -zoom -x 800 -y 400 %s &", video_path);
	
	system(path); // 执行播放命令
}

// 暂停播放/继续播放
void mplayer_pause()
{
	write_fifo("pause\n");
}
// 退出
void mplayer_quit()
{
	write_fifo("quit\n");
}

// 前进(正数)/后退（负数）
void mplayer_seek(int num)
{
	char cmd[50] = {0}; // 定义了一个字符串数组
	
	sprintf(cmd, "seek %d\n", num);
	
	write_fifo(cmd); // 执行播放命令
}

