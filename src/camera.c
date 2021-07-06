#include <stdio.h>   	//printf scanf
#include <fcntl.h>		//open write read lseek close  	 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "camera.h"
#include "api_v4l2.h"
#include "jpeglib.h"

struct jpg_data video_buf;//定义结构体变量

//初始化摄像头
void camera_init()
{
	linux_v4l2_yuyv_init("/dev/video7");
}

//开启摄像头捕捉
void camera_start()
{
	linux_v4l2_start_yuyv_capturing();
}

void camera_show(int x, int y)
{
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	show_video_data(x, y, video_buf.jpg_data, video_buf.jpg_size);
}

// 关闭摄像头
void camera_close()
{
	linux_v4l2_yuyv_quit();
}

