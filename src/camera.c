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

struct jpg_data video_buf;//����ṹ�����

//��ʼ������ͷ
void camera_init()
{
	linux_v4l2_yuyv_init("/dev/video7");
}

//��������ͷ��׽
void camera_start()
{
	linux_v4l2_start_yuyv_capturing();
}

void camera_show(int x, int y)
{
	linux_v4l2_get_yuyv_data(&video_buf);//��ȡ����ͷ��׽�Ļ���
	show_video_data(x, y, video_buf.jpg_data, video_buf.jpg_size);
}

// �ر�����ͷ
void camera_close()
{
	linux_v4l2_yuyv_quit();
}

