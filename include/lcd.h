#ifndef _LCD_H_
#define _LCD_H_

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

void open_fb0(void);
void close_fb0(void);
void clean_fb0(void);
void drwm_point(int x, int y, int color);
void show_bmp(int x, int y, char *bmp_path);
int show_video_data(unsigned int x,unsigned int y,char *pjpg_buf,unsigned int jpg_buf_size) ;
unsigned long file_size_get(const char *pfile_path);
int lcd_draw_jpg(unsigned int x,unsigned int y,const char *pjpg_path);

#endif