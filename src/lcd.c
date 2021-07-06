#include "lcd.h"
#include "jpeglib.h"
#include <unistd.h>
#include <stdlib.h>

#define LCD_WIDTH  			800
#define LCD_HEIGHT 			480
#define FB_SIZE				(LCD_WIDTH * LCD_HEIGHT * 4)

int g_fd;      // 屏幕文件的文件描述符
int *g_mmap;

static unsigned char g_color_buf[FB_SIZE]={0};

/**
*名称：open_fb0
*功能：打开开发板屏幕文件并且映射
*参数：无
*说明：无
**/
void open_fb0(void)
{
	g_fd = open("/dev/fb0", O_RDWR);
	
	g_mmap = mmap(NULL, 800*480*4, PROT_READ|PROT_WRITE, MAP_SHARED, g_fd, 0);
}

/**
*名称：close_fb0
*功能：关闭开发板屏幕文件并且映射
*参数：无
*说明：无
**/
void close_fb0(void)
{
	munmap(g_mmap, 800*480*4);
	close(g_fd);
}

/**
*名称：clean_fb0
*功能：清除开发板屏幕画面
*参数：无
*说明：无
**/
void clean_fb0(void)
{
	int i = 0;
	
	for(i=0; i<800*480; i++)
	{	
		*(g_mmap + i) = 0x00FFFFFF;
	}
}

/**
*名称：drwm_point
*功能：任意坐标地址绘点
*参数：
		x：绘点x轴坐标
		y: 绘点y轴坐标
		color：绘点的颜色
*说明：无
**/
void drwm_point(int x, int y, int color)
{
	*(g_mmap + y*800+x) = color;
}

/**
*名称：show_bmp
*功能：显示位置显示任意大小的BMP图片
*参数：
		x：图片起点x轴坐标
		y: 图片起点y轴坐标
		bmp_path：BMP图片的路径
*说明：无
**/
void show_bmp(int x, int y, char *bmp_path)
{
	int i = 0;
	int j = 0;
	
	int x_s = x;
	int y_s = y;
	
	int bmp_fd = -1;
	int bmp_high = 0;
	int bmp_width = 0;
	char bmp_buf[800*480*3] = {0};
	
	// 打开图片
	bmp_fd = open(bmp_path, O_RDWR);
	if(bmp_fd < 0)
	{
		printf("bmp_fd=%d\n", bmp_fd);
	}
	
	 printf("bmp_fd=%d\n", bmp_fd);
	 // 将图片数据读取到缓冲区
	 read(bmp_fd, bmp_buf, 54);
	 
	 // 获取高度和宽度
	 bmp_width  = bmp_buf[18];
	 bmp_width |= bmp_buf[19]<<8;
	 printf("bmp_width=%d\n", bmp_width);
	 
	 bmp_high  = bmp_buf[22];
	 bmp_high |= bmp_buf[23]<<8;
	 printf("bmp_high=%d\n", bmp_high);
	
	
	// 读取图片数据
	lseek(bmp_fd, 54, SEEK_SET);
	read(bmp_fd, bmp_buf, sizeof(bmp_buf));
	
	close(bmp_fd);
	j = 0;
	for(y_s=y; y_s<y+bmp_high; y_s++)
	{
		for(x_s=x; x_s<x+bmp_width; x_s++)
		{
			if(x_s > 800 || y_s > 480)
				continue;
			
			drwm_point(x_s, bmp_high-1-y_s+2*y, bmp_buf[j+0] | bmp_buf[j+1]<<8 | bmp_buf[j+2]<<16);
			j+=3;
		}	
	}
}

//获取jpg文件的大小
unsigned long file_size_get(const char *pfile_path)
{
	unsigned long filesize = -1;	
	struct stat statbuff;
	
	// 获取文件属性
	if(stat(pfile_path, &statbuff) < 0)
	{
		return filesize;
	}
	else
	{
		filesize = statbuff.st_size;
	}
	
	return filesize;
}


//显示摄像头捕捉
int show_video_data(unsigned int x,unsigned int y,char *pjpg_buf,unsigned int jpg_buf_size)  
{
	/*定义解码对象，错误处理对象*/
	struct 	jpeg_decompress_struct 	cinfo;
	struct 	jpeg_error_mgr 			jerr;	
	
	unsigned char 	*pcolor_buf = g_color_buf;
	char 	*pjpg;
	
	unsigned int 	i=0;
	unsigned int	color =0;
	//unsigned int	count =0;
	
	unsigned int 	x_s = x;
	unsigned int 	x_e ;	
	unsigned int 	y_e ;
	
	pjpg = pjpg_buf;

	/*注册出错处理*/
	cinfo.err = jpeg_std_error(&jerr);

	/*创建解码*/
	jpeg_create_decompress(&cinfo);

	/*直接解码内存数据*/		
	jpeg_mem_src(&cinfo,pjpg,jpg_buf_size);
	
	/*读文件头*/
	jpeg_read_header(&cinfo, TRUE);

	/*开始解码*/
	jpeg_start_decompress(&cinfo);	
	
	x_e	= x_s+cinfo.output_width;
	y_e	= y  +cinfo.output_height;	

	/*读解码数据*/
	while(cinfo.output_scanline < cinfo.output_height )
	{		
		pcolor_buf = g_color_buf;
			
		/* 读取jpg一行的rgb值 */
		jpeg_read_scanlines(&cinfo,&pcolor_buf,1);
		
		for(i=0; i<cinfo.output_width; i++)
		{
			/* 获取rgb值 */
			color = 		*(pcolor_buf+2);
			color = color | *(pcolor_buf+1)<<8;
			color = color | *(pcolor_buf)<<16;
			
			/* 显示像素点 */
			drwm_point(x,y,color);
			
			pcolor_buf +=3;
			
			x++;
		}
		
		/* 换行 */
		y++;			
		
		x = x_s;
		
	}		
			
	/*解码完成*/
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return 0;
}


//显示正常jpg图片
int lcd_draw_jpg(unsigned int x,unsigned int y,const char *pjpg_path)  
{
	/*定义解码对象，错误处理对象*/
	struct 	jpeg_decompress_struct 	cinfo;
	struct 	jpeg_error_mgr 			jerr;	
	
	unsigned char 	*pcolor_buf = g_color_buf;
	char 	*pjpg;
	
	unsigned int 	i=0;
	unsigned int	color =0;
	//unsigned int	count =0;
	
	unsigned int 	x_s = x;
	unsigned int 	x_e ;	
	unsigned int 	y_e ;
	unsigned int	y_n	= y;
	unsigned int	x_n	= x;
	
			 int	jpg_fd;
	unsigned int 	jpg_size;

	if(pjpg_path!=NULL)
	{
		/* 申请jpg资源，权限可读可写 */	
		jpg_fd=open(pjpg_path,O_RDWR);
		
		if(jpg_fd == -1)
		{
		   printf("open %s error\n",pjpg_path);
		   
		   return -1;	
		}	
		
		/* 获取jpg文件的大小 */
		jpg_size=file_size_get(pjpg_path);	
		if(jpg_size<3000)
			return -1;
		printf("jpg_size=%d\n", jpg_size);
		
		/* 为jpg文件申请内存空间 */	
		pjpg = malloc(jpg_size);

		/* 读取jpg文件所有内容到内存 */		
		read(jpg_fd,pjpg,jpg_size);
	}
	else
		return -1;

	/*注册出错处理*/
	cinfo.err = jpeg_std_error(&jerr);

	/*创建解码*/
	jpeg_create_decompress(&cinfo);

	/*直接解码内存数据*/		
	jpeg_mem_src(&cinfo,pjpg,jpg_size);
	
	/*读文件头*/
	jpeg_read_header(&cinfo, TRUE);

	/*开始解码*/
	jpeg_start_decompress(&cinfo);	
	
	
	x_e	= x_s +cinfo.output_width; // 图片宽度
	y_e	= y  +cinfo.output_height;	// 图片的高度

	/*读解码数据*/
	while(cinfo.output_scanline < cinfo.output_height )
	{		
		pcolor_buf = g_color_buf;
		
		/* 读取jpg一行的rgb值 */
		jpeg_read_scanlines(&cinfo,&pcolor_buf,1);
		
		// 将一行数据显示到屏幕
		for(i=0; i<cinfo.output_width; i++)
		{	
			/* 获取rgb值 */
			color = 		*(pcolor_buf+2);
			color = color | *(pcolor_buf+1)<<8;
			color = color | *(pcolor_buf)<<16;	
			
			/* 显示像素点 */
			drwm_point(x_n,y_n,color);
			
			pcolor_buf +=3;
			
			x_n++;
		}
		
		/* 换行 */
		y_n++;			
		
		x_n = x_s;		
	}		
			
	/*解码完成*/
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	if(pjpg_path!=NULL)
	{
		/* 关闭jpg文件 */
		close(jpg_fd);	
		
		/* 释放jpg文件内存空间 */
		free(pjpg);		
	}

	return 0;
}


