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


#define JPG_PATH "./result.jpg"
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

// 拍照
/*void camera_cap()
{
	char camera_jpg_cmd[50]={0};
	char *photo_path = "/root/photo";
	char camera_jpg_name[50]={0};	
	static int count = 0;
	int tmp_fd = -1; 
	// 如果文件不存在就创建并打开	
	memset(camera_jpg_cmd,0,sizeof(camera_jpg_cmd));
	sprintf(camera_jpg_cmd,"%s/camera%d.jpg",photo_path,count);
	tmp_fd = open(camera_jpg_cmd, O_RDWR|O_CREAT, 0777);
	if(tmp_fd < 0)
	{
		printf("open tmp_fd fail!\n");
	}
	printf("tmp_fd = %d\n", tmp_fd);
	
	// 写入jpg数据
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	
	// printf("video_buf.jpg_size = %d\n", video_buf.jpg_size);
	write(tmp_fd, video_buf.jpg_data, video_buf.jpg_size);
	// lcd_draw_jpg
	// 关闭图片文件
	close(tmp_fd);
	memset(camera_jpg_name,0,sizeof(camera_jpg_name));
	sprintf(camera_jpg_name,"camera%d.jpg",count++);
	P_DOUBLE_NODE camera_new = new_node(camera_jpg_name);	//创建新节点		
	add_node(camera_new,photo_list);						//插入新节点
}
*/
// 拍照
void camera_cap()
{
	int tmp_fd = -1; 
	// 如果文件不存在就创建并打开
	tmp_fd = open(JPG_PATH, O_RDWR|O_CREAT, 0777);
	if(tmp_fd < 0)
	{
		printf("open tmp_fd fail!\n");
	}
	printf("tmp_fd = %d\n", tmp_fd);

	usleep(1000*100);
	// 写入jpg数据
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	linux_v4l2_get_yuyv_data(&video_buf);//获取摄像头捕捉的画面
	
	usleep(1000*100);
	// printf("video_buf.jpg_size = %d\n", video_buf.jpg_size);
	write(tmp_fd, video_buf.jpg_data, video_buf.jpg_size);
	usleep(1000*10);
	// lcd_draw_jpg
	// 关闭图片文件
	close(tmp_fd);
}