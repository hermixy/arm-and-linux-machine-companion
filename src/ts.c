#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/input.h>

#include "ts.h"

int ts_fd; // 触摸屏文件的文件描述符

// 打开触摸屏文件
void ts_open()
{
	//打开触摸屏设备文件
    ts_fd = open("/dev/input/event0", O_RDWR);
    if(ts_fd == -1)
    {
        perror("open ts_fd failed");
    }
}

int ts_getxy(int *x, int *y)
{
	static int x_flag = 0;
	static int y_flag = 0;
	int i = 50;
	
	struct input_event ts; // 结构体变量，存储坐标数据
	while(i--)
	{	
		// 读取文件数据
		read(ts_fd, &ts, sizeof(ts));
		// 判断坐标值
		if(ts.type == EV_ABS) // 触摸屏
		{
			if(ts.code == ABS_X) // x轴
			{
				printf("x = %d\n", ts.value);
				*x = ts.value;
				x_flag = 1;
			}
			
			if(ts.code == ABS_Y)
			{
				printf("y = %d\n", ts.value);
				*y = ts.value;
				y_flag = 1;
			}
		}
		if(x_flag && y_flag)
			break;
	}
		// 如果得到了x和y坐标
		if(x_flag && y_flag)
		{
			printf("x = %d y = %d\n", *x, *y);
			x_flag = 0;
			y_flag = 0;
			return 1;	
		}	
		else
		{
			return -1;
		} 
			

    //close(ts_fd);
}

void ts_close()
{
	close(ts_fd);
}

int my_ts_getxy(int *x, int *y)
{
	int x_flag = 0;
	int y_flag = 0;
	
	struct input_event ts; // 结构体变量，存储坐标数据
	
	while(1)
	{
		// 读取文件数据
		read(ts_fd, &ts, sizeof(ts));
		
		// 判断坐标值
		if(ts.type == EV_ABS) // 触摸屏
		{
			if(ts.code == ABS_X) // x轴
			{
				//printf("x = %d\n", ts.value);
				*x = ts.value;
				x_flag = 1;
			}
			
			if(ts.code == ABS_Y)
			{
				//printf("y = %d\n", ts.value);
				*y = ts.value;
				y_flag = 1;
			}
		}
		
		// 如果得到了x和y坐标
		if(x_flag && y_flag)
			break;

	}

    //close(ts_fd);
}