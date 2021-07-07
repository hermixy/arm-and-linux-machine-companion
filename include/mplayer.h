#ifndef __MPLAYER_H__
#define __MPLAYER_H__

int fifo_init();
int write_fifo(char *ctrl);
void mplayer_start(char *video_path); // 播放视频
void mplayer_pause();
void mplayer_seek(int num);
void mplayer_volume(int num);
void mplayer_quit();
void camera_cap();

#endif