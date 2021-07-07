/*
************************************************************************************************************************
*                                                 基于科大讯飞机器伴侣
*
*					     本代码参考《基于嵌入式实时操作系统的程序设计技术》,《UCOS-III内核实现与开发指南》
*
*
* File    : main.c
* By      : wcf
* Version : V0.0.1
*
* 更新记录:
* ---------------
*				-2021.06.29- V0.0.1代码模板初步搭建完成
*				-2021.07.07- V1.0.0代码基本功能完成
*
*
*
************************************************************************************************************************
* Note(s) :NULL
*
*       
************************************************************************************************************************
*/
/*
*********************************************************************************************************
*                                              头文件
*********************************************************************************************************
*/
#include <stdio.h>
#include <stdint.h>						//数据类型头文件
#include <stdlib.h>		
#include <string.h>				
#include <pthread.h>
#include <unistd.h>		
#include <sys/types.h>					//	进程头文件
#include <sys/ipc.h>
#include <sys/msg.h>
#include <mqueue.h>
#include <dirent.h>
#include <errno.h>
#include "mplayer.h"
#include "lcd.h"
#include "ts.h"		
#include "camera.h"
#include "socket.h"

/*
*********************************************************************************************************
*                                                 宏定义
*********************************************************************************************************
*/
#define MSG_BUFFER_SIZE 4

/*
*********************************************************************************************************
*                                                线程声明
*********************************************************************************************************
*/
//pthread_t TaskDisplayPthread;				//显示任务线程
pthread_t recv_client_data_tid;				//后台获取客户端返回值线程
/*
*********************************************************************************************************
*                                                全局变量定义
*********************************************************************************************************
*/
uint8_t Screen;									//画面编号
uint8_t	Count;
uint8_t camera_show_flag = 0;					//相机等待后台坐标获取
uint8_t wait_load_flag = 0;						//等待过渡动画完成标志位置
uint8_t recv_client_data_flag = 0;				//录音等待抓换完成
uint32_t recv_client_data_sm = 0;				//录音识别结果
int		my_get_touch_return;					//相机后台坐标获取判断值

typedef struct DOUBELNODE{
	char *data;
	struct DOUBELNODE *Pnext;
	struct DOUBELNODE *Pprev;
}DOUBLE_NODE,*P_DOUBLE_NODE;

int16_t	ScreenKey0[1][4] = {			//登录界面
							{0,0,799,479},//全屏幕范围
};

int16_t	ScreenKey1[6][4] = {			//"操作选择"画面（主菜单）的按钮表
							{96,204,148,415},		//键码0：相册
							{197,192,323,416},		//键码1：音频
							{348,187,452,414},		//键码2：录音
							{470,179,599,415},		//键码3：视频
							{631,190,747,416},		//键码4：相机					
							{0,0,799,479}			//键码5：全屏范围选择（不做处理，为default状态）
};

int16_t	ScreenKey2[7][4] = {			//"相册"画面的按钮表
							{1,1,59,232},			//键码0：上一张
							{700,201,785,475},		//键码1：下一张
							{650,0,785,200},		//键码2：返回
							{61,87,271,341},		//键码3：第一张
							{293,82,503,334},		//键码4：第二张
							{519,81,650,330},		//键码5：第三张						
							{0,0,799,479}			//键码6：全屏范围选择（不做处理，为default状态）
};

int16_t	ScreenKey3[5][4] = {			//"音频"画面的按钮表
							{401,356,449,429},		//键码0：上一首
							{537,354,585,429},		//键码1：下一首
							{650,0,779,100},		//键码2：返回
							{469,371,520,423},		//键码3：暂停，播放					
							{0,0,799,479}			//键码4：全屏范围选择（不做处理，为default状态）
};

int16_t	ScreenKey4[5][4] = {			//"录音"画面的按钮表
							{295,301,355,357},		//键码0：开始
							{396,302,453,355},		//键码1：暂停
							{650,0,779,100},		//键码2：返回
							{500,297,559,355},		//键码3：结束	
							{0,0,799,479}			//键码4：全屏范围选择（不做处理，为default状态）
};

int16_t	ScreenKey5[7][4] = {			//"视频"画面的按钮表
							{326,408,364,475},		//键码0：上一个
							{436,402,481,475},		//键码1：下一个
							{650,0,779,479},		//键码2：返回
							{494,402,560,474},		//键码3：快进
							{248,406,312,475},		//键码4：回退
							{383,350,421,479},		//键码5：播放														
							{0,0,799,479}			//键码6：全屏范围选择（不做处理，为default状态）
};

int16_t	ScreenKey6[4][4] = {			//"相机"画面的按钮表
							{700,350,779,480},		//键码0：相册
							{660,201,795,280},		//键码1：拍照
							{650,0,779,150},		//键码2：返回
							{0,0,799,479}			//键码3：全屏范围选择（不做处理，为default状态）
};

int16_t	ScreenKey7[7][4] = {			//"相机相册"画面的按钮表
							{1,210,62,232},			//键码0：上一张
							{735,201,785,230},		//键码1：下一张
							{650,0,779,100},		//键码2：返回
							{61,87,271,341},		//键码3：第一张
							{293,82,503,334},		//键码4：第二张
							{519,81,718,330},		//键码5：第三张						
							{0,0,799,479}			//键码6：全屏范围选择（不做处理，为default状态）
};

/*
*********************************************************************************************************
*                                              任务函数名声明
*********************************************************************************************************
*/
void NewScree(void);						//屏幕刷新函数
int8_t	TouchKey(int16_t x,int16_t y);		//触摸屏坐标返回函数
static void	AppTaskDisplay(void);			//显示任务
static void AppTaskTouch(void);				//监控任务
void *MyGetTouch(void *arg);				//后台获取线程坐标
void *recv_client_data(void *arg);			//后台获取客户端是否接入并进行相应判断
void pthread_clean_up(void *arg);			//工具函数，清理线程时调用
void show_cartoon();
P_DOUBLE_NODE list_init(char *new_data);
P_DOUBLE_NODE new_node(char *new_data);
void add_node(P_DOUBLE_NODE new,P_DOUBLE_NODE list);


/*
========================================================================================================================
*                                                       主函数
========================================================================================================================
*/
int main(void)
{
	// 启动服务器，等待客户端连接
	socket_server_init(8888); // 初始化TCP服务器
	socket_accept(); // 等待客户端连接	
	// 创建线程
	//	pthread_create(&TaskDisplayPthread, NULL, DisplayPthread, NULL);	
	//pthread_create(&recv_client_data_tid, NULL, recv_client_data, NULL);
		
	pid_t result;	
	result = fork();
	ts_open();							// 打开触摸屏	
	open_fb0();	
	fifo_init();

	if(result == -1)
	{
		printf("fork error\n");
		exit(1);
	}
	if(result == 0)//执行子进程
	{
		AppTaskDisplay();
	}
	else//执行父进程
	{
		AppTaskTouch();
	}
	return 0;
}

/*
------------------------------------------------------------------------------------------------------------------------
*                                                    监控任务的实现
------------------------------------------------------------------------------------------------------------------------
*/
static void AppTaskTouch(void )
{
//	
	static uint8_t i = 0;
	uint8_t		KEY;
	uint32_t	SM;
	int music_start_stop_flag = 0;
	int video_start_stop_flag = 0;	
	int 		touch_send_qid;			//消息队列ID
	int			touch_recive_qid;
	int 		send_length, recv_length;	
	key_t 		touch_send_key;			//监控任务的消息队列标识符
	key_t		touch_recive_key;
	struct message
	{
		long msg_type;
		uint32_t msg_sm[MSG_BUFFER_SIZE];
	};
	struct message touch_send_msg;
	struct message touch_recive_msg;
	send_length = sizeof(struct message) - sizeof(long );		
	recv_length = sizeof(struct message) - sizeof(long );		
	printf("head touch:%d\n",i++);
	/* 
	**创建消息到消息队列 
	*/
	if((touch_send_key = ftok("/",'a')) == -1)
	{
		printf("touch_send ftok error\n");
		exit(1);
	}
	if((touch_send_qid = msgget(touch_send_key,IPC_CREAT|0666)) == -1)
	{
		printf("touch_send msgget error\n");
		exit(1);
	}
	touch_send_msg.msg_type = (long int)2;
	/* 
	**接收消息队列的消息 
	*/
	if((touch_recive_key = ftok("/root",'b')) == -1)
	{
		printf("touch_recive ftok error\n");
		exit(1);
	}
	if((touch_recive_qid = msgget(touch_recive_key,IPC_CREAT|0666)) == -1)
	{
		printf("touch_recive msgget error\n");
		exit(1);
	}	
	
	//发送第一条消息显示零号画面
	SM = 0x01<<24;												 //显示零号画面即系统封面	
	touch_send_msg.msg_sm[0] = SM;
	if((msgsnd(touch_send_qid,&touch_send_msg,send_length,0)) < 0)
	{
		printf("touch_send message posted\n");
		exit(1);
	}
	printf("touch_send_msg:%x\n",touch_send_msg.msg_sm[0]);	
	sleep(3);
//	
	while(1)
	{	
		printf("touch:%d\n",i++);
		/*
		**延时
		*/
		//sleep(1);
		/* 
		**发布消息到消息队列 发送给显示任务，查询触摸屏,触摸屏根据发来的信息，进行触摸屏查询
		**查询到不同的地方更新键码
		*/
		touch_send_msg.msg_sm[0] = 0x55<<24;
		if((msgsnd(touch_send_qid,&touch_send_msg,send_length,0)) < 0)
		{
			printf("touch_send message posted\n");
			exit(1);
		}	
		printf("touch_send_msg:%x\n",touch_send_msg.msg_sm[0]);				
		/*
		**查询获得键码
		*/		
		memset(touch_recive_msg.msg_sm,0,16);	
		if(msgrcv(touch_recive_qid,(void *)&touch_recive_msg,recv_length,(long)0,0) < 0)
		{
			printf("touch_recive message error\n");
			printf("%s\n", strerror(errno));
			exit(1);
		}
		printf("touch_recive_msg:%x\n",touch_recive_msg.msg_sm[0]);	
		SM = touch_recive_msg.msg_sm[0];
		KEY = (uint8_t)(SM & 0x000000ff);
		if(KEY == 0x5a)	continue;					//未触摸，再延时，此处作用不大，会在显示任务中一直等待触摸，有触摸才会返回		
		SM = 0;
		Screen = touch_recive_msg.msg_sm[1];
		printf("touch Screen :%d \n",Screen);
//		
		switch(Screen)								//按画面分别处理（第一层）
		{
			case	0:								//封面
					/*按钮处理第二层*/
					SM = 0x01<<24;
					SM |= 0x01<<16;					//进入操作选择画面，更新屏幕编号为1
					Count = 0;						//结束画面倒计时
					break;
			case	1:								//“操作选择”画面（主菜单）
					/*按钮处理第二层*/
					switch(KEY)						//按键码分别处理
					{
						case	0:					//相册
								SM = 0X01<<24;		//“显示相册”命令码
								SM |= 0X02<<16;		//进入“相册”画面,更新屏幕编号
								break;
						case	1:					//音频
								SM = 0X01<<24;		//“显示音频”命令码
								SM |= 0X03<<16;		//进入“选择音频”画面,更新屏幕编号
								break;
						case	2:					//录音
								SM = 0X01<<24;		//“显示录音”命令码
								SM |= 0X04<<16;		//进入“录音”画面,更新屏幕编号
								break;
						case	3:					//视频
								SM = 0X01<<24;		//“显示视频”命令码
								SM |= 0X05<<16;		//进入“视频”画面,更新屏幕编号
								break;		
						case	4:					//相机
								SM = 0X6a<<24;		//“显示相机”命令码
								SM |= 0X06<<16;		//进入“相机”画面,更新屏幕编号
								break;	
						case    0xff:				//ff情况则是刷新背景
								SM = 0x01<<24;
								SM |= 0x01<<16;	
								break;															
						default:					//默认情况是录音					
								SM = 0x3a<<24;
								SM |= 0x04<<16;								
								printf("bakcground default\n");
								break;
					}
					break;							
			case	2:								//“相册”画面
					/*按钮处理第二层*/
					switch(KEY)						//按键码分别处理
					{					
						case	0:					//“上一张”
								printf("photo 0\n");
								SM = 0x1a<<24;
								SM |= 0x02<<16;									
								break;
						case	1:					//“下一张”			
								printf("photo 1\n");			
								SM = 0x1b<<24;
								SM |= 0x02<<16;	
								break;
						case	2:					//“返回”
								SM = 0x01<<24;
								SM |= 0x01<<16;
								printf("photo 2\n");									
								break;
						case	3:					//“第一张”
								printf("photo 3\n");
								SM = 0x1c<<24;
								SM |= 0x02<<16;									
								break;
						case	4:					//“第二张”			
								printf("photo 4\n");
								SM = 0x1d<<24;
								SM |= 0x02<<16;																									
								break;
						case	5:					//“第三张”
								printf("photo 5\n");
								SM = 0x1e<<24;
								SM |= 0x02<<16;								
								break;							
						default:
								printf("photo default\n");
								break;
					}
					break;
			case	3:								//“音乐”画面
					/*按钮处理第二层*/
					switch(KEY)						//按键码分别处理
					{
						case	0:					//“上一首”按钮
								SM = 0x2d<<24;
								SM |= 0x03<<16;									
								break;
						case	1:					//“下一首”按钮	
								SM = 0x2e<<24;
								SM |= 0x03<<16;	
								break;
						case	2:					//“返回”按钮
								SM = 0x2f<<24;
								SM |= 0x01<<16;	
								music_start_stop_flag = 0;						
								break;
						case	3:					//“暂停/播放”按钮
								if(music_start_stop_flag == 0)
								{
									SM = 0x2a<<24;
									SM |= 0x03<<16;	
									music_start_stop_flag = 2;									
								}
								else if(music_start_stop_flag == 1)//播放
								{
									SM = 0x2b<<24;
									SM |= 0x03<<16;	
									music_start_stop_flag = 2;
								}		
								else if(music_start_stop_flag == 2)//停止
								{
									SM = 0x2c<<24;
									SM |= 0x03<<16;		
									music_start_stop_flag = 1;								
								}	
								break;
						default:
								break;
					}
					break;
			case	4:								//"录音"画面
					/*按钮处理第二层*/
					switch(KEY)						//按键码分别处理
					{
						case	0:					//“开始”按钮
								SM = 0x3a<<24;
								SM |= 0x04<<16;									
								break;
						case	1:					//“暂停”按钮													
								break;
						case	2:					//“返回”按钮
								SM = 0x01<<24;
								SM |= 0x01<<16;							
								break;
						case	3:					//“结束”按钮									
								break;
						default:
								break;
					}					
					/*按钮处理第二层*/
					break;
			case	5:								//"视频"画面
					switch(KEY)						//按键码分别处理
					{
						case	0:					//“上一个视频”按钮
								SM = 0x41<<24;
								SM |= 0x05<<16;							
								break;
						case	1:					//“下一个视频”按钮	
								SM = 0x42<<24;
								SM |= 0x05<<16;																			
								break;
						case	2:					//“返回”按钮
								SM = 0x4d<<24;
								SM |= 0x01<<16;	
								video_start_stop_flag = 0;
								break;
						case	3:					//“快进”按钮	
								SM = 0x4e<<24;
								SM |= 0x05<<16;									
								break;
						case	4:					//“回退”按钮
								SM = 0x4f<<24;
								SM |= 0x05<<16;	
								break;
						case	5:					//“播放”按钮
								if(video_start_stop_flag == 0)//播放
								{
									SM = 0x4a<<24;
									SM |= 0x05<<16;	
									video_start_stop_flag = 2;									
								}
								else if(video_start_stop_flag == 1)//播放
								{
									SM = 0x4b<<24;
									SM |= 0x05<<16;	
									video_start_stop_flag = 2;
								}		
								else if(video_start_stop_flag == 2)//停止播放
								{
									SM = 0x4c<<24;
									SM |= 0x05<<16;		
									video_start_stop_flag = 1;								
								}						
								break;								
						default:
								break;
					}
					break;					
					/*按钮处理第二层*/					
			case	6:								//“相机”画面			
					/*按钮处理第二层*/	
					switch(KEY)						//按键码分别处理
					{
						case	0:					//“相册”按钮
								SM = 0x01<<24;
								SM |= 0x07<<16;	
								break;
						case	1:					//“拍照”按钮
								SM = 0x6b<<24;
								SM |= 0x06<<16;																						
								break;
						case	2:					//“返回”按钮
								SM = 0x01<<24;
								SM |= 0x01<<16;	
								break;
						default:	
								SM = 0x6a<<24;
								SM |= 0x06<<16;	
								break;
					}	
					break;
			case	7:								//“相机相册”画面
					/*按钮处理第二层*/
					switch(KEY)						//按键码分别处理
					{					
						case	0:					//“上一张”
								printf("photo 0\n");
								SM = 0x1a<<24;
								SM |= 0x07<<16;	
								break;
						case	1:					//“下一张”			
								printf("photo 1\n");			
								SM = 0x1b<<24;
								SM |= 0x07<<16;	
								break;
						case	2:					//“返回”
								SM = 0X6a<<24;		//“显示相机”命令码
								SM |= 0X06<<16;		//进入“相机”画面,更新屏幕编号
								printf("photo 2\n");									
								break;
						case	3:					//“第一张”
								printf("photo 3\n");
								SM = 0x1c<<24;
								SM |= 0x07<<16;									
								break;
						case	4:					//“第二张”			
								printf("photo 4\n");
								SM = 0x1d<<24;
								SM |= 0x07<<16;																									
								break;
						case	5:					//“第三张”
								printf("photo 5\n");
								SM = 0x1e<<24;
								SM |= 0x07<<16;								
								break;							
						default:
								printf("photo default\n");
								break;
					}
					break;								
			default:								//未定义画面不处理
					break;
			
		}
		/*向消息队列发送短消息，要求显示终端进行相应的显示操作*/
		if(SM)
		{
			/* 
			**发布消息到消息队列 
			*/
			touch_send_msg.msg_sm[0] = SM;
			if((msgsnd(touch_send_qid,&touch_send_msg,send_length,0)) < 0)
			{
				printf("touch_send message posted\n");
				exit(1);
			}
			printf("touch_send_msg:%x\n",touch_send_msg.msg_sm[0]);				
		}

	}
}

/*
------------------------------------------------------------------------------------------------------------------------
*                                       显示任务的实现AppTaskDisplay(void )
------------------------------------------------------------------------------------------------------------------------
*/

static void	AppTaskDisplay(void )
{
//
	static uint8_t i = 0;
	char music_cmd[50]={0};	
	char photo_cmd[50]={0};		
	char video_cmd[50]={0};	
	/*
	**拍照显示相关声明
	*/
	char camera_jpg_name[50]={0};		
	char *cemera_photo_path = "/root/photo";
	char camera_jpg_cmd[50]={0};	
	struct jpg_data video_buf;	
	static int camera_photo_count = 0;		
	int tmp_fd = -1; 
	/*
	*/	
	uint32_t	SM = 0;										//短消息
	int			err;										//触摸屏幕函数返回值接收
	int 		display_send_qid;							//消息队列ID，根据此ID识别消息队列
	int 		display_recive_qid;       
	int			X,Y;									  	//触摸坐标
	int 		send_length, recv_length;
	key_t 		display_send_key;							//显示任务的消息队列标识符
	key_t		display_recive_key;
	struct message
	{
		long msg_type;
		uint32_t msg_sm[MSG_BUFFER_SIZE];
	};
	struct message display_send_msg;	
	struct message display_recive_msg;		
	send_length = sizeof(struct message) - sizeof(long);	
	recv_length	= sizeof(struct message) - sizeof(long);			
	printf("head display:%d\n",i++);			
	pthread_t my_get_touch_tid;			// 创建线程，后台获取点击的坐标，以便退出相机显示界面			
	/* 
	**接收消息队列的消息 
	*/
	if((display_recive_key = ftok("/",'a')) == -1)
	{
		printf("display_recive ftok error\n");
		exit(1);
	}
	if((display_recive_qid = msgget(display_recive_key,IPC_CREAT|0666)) == -1)
	{
		printf("display_recive msgget error\n");
		exit(1);
	}	
	/* 
	**创建消息队列 
	*/
	if((display_send_key = ftok("/root",'b')) == -1)
	{
		printf("display_send ftok error\n");
		exit(1);
	}
	if((display_send_qid = msgget(display_send_key,IPC_CREAT|0666)) == -1)
	{
		printf("display_send msgget error\n");
		exit(1);
	}
	display_send_msg.msg_type = (long)2;	

	/*音乐循环链表*/	
	P_DOUBLE_NODE music_list = list_init("music3.mp3");	
	char *music_path = "/root";
	DIR *music_dp = opendir(music_path);
	struct dirent *p_music;
	
	while(p_music = readdir(music_dp))
	{
		if(p_music->d_type == DT_REG)
		{
			if(strstr(p_music->d_name,".mp3"))					//判断是否为.mp3文件
			{
				P_DOUBLE_NODE new1 = new_node(p_music->d_name);	//创建新节点
				add_node(new1,music_list);						//插入新节点
			}
		}
	}
	P_DOUBLE_NODE music_head = music_list->Pnext;
	
	/*相册循环链表*/
	P_DOUBLE_NODE photo_list = list_init("photo1.jpg");	
	char *photo_path = "/root/photo";
	DIR *photo_dp = opendir(photo_path);
	struct dirent *p_photo;
	
	while(p_photo = readdir(photo_dp))
	{
		if(p_photo->d_type == DT_REG)
		{
			if(strstr(p_photo->d_name,".jpg"))					//判断是否为.jpg文件
			{
				P_DOUBLE_NODE new2 = new_node(p_photo->d_name);	//创建新节点
				add_node(new2,photo_list);						//插入新节点
			}
		}
	}
	P_DOUBLE_NODE photo_head = photo_list->Pnext;


	/*视频循环链表*/
	P_DOUBLE_NODE video_list = list_init("video1.avi");	
	char *video_path = "/root";
	DIR *video_dp = opendir(video_path);
	struct dirent *p_video;
	
	while(p_video = readdir(video_dp))
	{
		if(p_video->d_type == DT_REG)
		{
			if(strstr(p_video->d_name,".avi"))					//判断是否为.jpg文件
			{
				P_DOUBLE_NODE new3 = new_node(p_video->d_name);	//创建新节点
				add_node(new3,video_list);						//插入新节点
			}
		}
	}
	P_DOUBLE_NODE video_head = video_list->Pnext;
		
	/*清屏，初始化设置*/
	// 打开触摸屏
	//ts_open();
	//open_fb0();
	clean_fb0();
	// 关闭触摸屏
	//ts_close();
	//close_fb0();
//	
	while(1)
	{
		printf("display:%d\n",i++);
		/*
		**不断接收监控任务发来得消息，根据消息变更屏幕信息
		*/
		memset(display_recive_msg.msg_sm, 0, 16);		
		if(msgrcv(display_recive_qid,(void *)&display_recive_msg,recv_length,(long)0,0) < 0)
		{
			printf("display_recive message error\n");
			printf("%s\n", strerror(errno));
			exit(1);
		}	
		printf("display_recive_msg:%x\n",display_recive_msg.msg_sm[0]);			
		SM = display_recive_msg.msg_sm[0];
		printf("sm:%x\n",SM);
		/*从消息队列中获取短消息*/
//		
		switch((SM>>24)&0xff)
		{
			case	0x01:								//显示新的画面
					Screen = (SM>>16&0xff);				//保存新的画面编号
					NewScree();							//显示新的画面
					break;
			case	0x02:								//保留
				    printf("case 2");
					break;	
			case	0x03:								//保留
					
					break;
			case	0x04:								//
					
					break;
			case	0x05:								//
					
					break;

														//相册界面预留
			case	0x1a:								//“上一张”
					photo_head = photo_head->Pprev;	
					if(photo_head == photo_list)
					{
						photo_head = photo_head->Pprev;
					}					
					memset(photo_cmd,0,sizeof(photo_cmd));
					sprintf(photo_cmd,"%s/%s",photo_path,photo_head->data);
					lcd_draw_jpg(0,0,photo_cmd);
					break;	
			case	0x1b:								//“下一张”			
					photo_head = photo_head->Pnext;	
					if(photo_head == photo_list)
					{
						photo_head = photo_head->Pnext;
					}						
					memset(photo_cmd,0,sizeof(photo_cmd));
					sprintf(photo_cmd,"%s/%s",photo_path,photo_head->data);
					lcd_draw_jpg(0,0,photo_cmd);
					break;
			case	0x1c:								//“第一张”	
					//open_fb0();
					lcd_draw_jpg(0,0,"./photo/photo1.jpg");
					//close_fb0();		
					//ts_open();
					while(ts_getxy(&X, &Y) == -1)
					{
						;
					}
					NewScree();							//显示新的画面					
					// 关闭触摸屏
					//ts_close();									
					break;					
			case	0x1d:								//“第二张”
					//open_fb0();
					lcd_draw_jpg(0,0,"./photo/photo2.jpg");
					//close_fb0();
					//ts_open();
					while(ts_getxy(&X, &Y) == -1)
					{
						;
					}
					NewScree();							//显示新的画面					
					// 关闭触摸屏
					//ts_close();											
					break;
			case	0x1e:								//“第三张”
					//open_fb0();
					lcd_draw_jpg(0,0,"./photo/photo3.jpg");
					//close_fb0();
					//ts_open();
					while(ts_getxy(&X, &Y) == -1)
					{
						;
					}
					NewScree();							//显示新的画面					
					// 关闭触摸屏
					//ts_close();											
					break;					
			case	0x1f:								//预留
					
					break;	

														//音乐界面预留
			case	0x2a:								//音乐播放	
					system("killall -9 madplay");									
					if(music_head == music_list)
					{
						music_head = music_head->Pnext;
					}	
					memset(music_cmd,0,sizeof(music_cmd));
					sprintf(music_cmd,"madplay %s/%s -r &",music_path,music_head->data);									
					system(music_cmd);	
					show_cartoon();		
					lcd_draw_jpg(0,0,"./music.jpg");															
					break;	
			case	0x2b:								//音乐继续
					system("killall -CONT madplay &"); 	
					show_cartoon();	
					lcd_draw_jpg(0,0,"./music.jpg");											
					break;
			case	0x2c:								//音乐停止
					system("killall -STOP madplay &");		
					show_cartoon();	
					lcd_draw_jpg(0,0,"./music.jpg");												
					break;					
			case	0x2d:								//上一曲						
					system("killall -9 madplay");
					music_head = music_head->Pprev;	
					if(music_head == music_list)
					{
						music_head = music_head->Pprev;
					}						
					memset(music_cmd,0,sizeof(music_cmd));
					sprintf(music_cmd,"madplay %s/%s -r &",music_path,music_head->data);
					system(music_cmd);
					break;
			case	0x2e:								//下一曲				
					system("killall -9 madplay");
					music_head = music_head->Pnext;	
					if(music_head == music_list)
					{
						music_head = music_head->Pnext;
					}					
					memset(music_cmd,0,sizeof(music_cmd));
					sprintf(music_cmd,"madplay %s/%s -r &",music_path,music_head->data);
					system(music_cmd);					
					break;					
			case	0x2f:								//退出
					Screen = (SM>>16&0xff);				//保存新的画面编号
					NewScree();							//显示新的画面					
					break;		

														//录音机界面预留
			case	0x3a:								//录音开始
					socket_send_file();

					char rec_rslt[1024] = {0};
					socket_recv_ack(rec_rslt, sizeof(rec_rslt));
					
					if(strstr(rec_rslt, "打开相册"))
					{
						recv_client_data_sm = 0X00<<24;				//更新KEY值
						recv_client_data_sm |= 0X01<<16;			//更新屏幕编号	
						printf("recv_client_data_sm = %x\n", recv_client_data_sm);		
					}
					else if(strstr(rec_rslt, "打开音乐"))
					{
						recv_client_data_sm = 0X01<<24;			
						recv_client_data_sm |= 0X01<<16;				
					}
					else if(strstr(rec_rslt, "播放音乐"))
					{		
						recv_client_data_sm = 0X03<<24;				//更新KEY值
						recv_client_data_sm |= 0X03<<16;			//更新屏幕编号	
					}
					else if(strstr(rec_rslt, "播放视频"))
					{
						recv_client_data_sm = 0X05<<24;			
						recv_client_data_sm |= 0X05<<16;
					}						
					else if(strstr(rec_rslt, "打开视频"))
					{
						recv_client_data_sm = 0X03<<24;				
						recv_client_data_sm |= 0X01<<16;					
					}				
					else if(strstr(rec_rslt, "打开相机"))
					{
						recv_client_data_sm = 0X04<<24;				
						recv_client_data_sm |= 0X01<<16;					
					}
					else
					{
						printf("无效输入\n");
					}		

					usleep(1000*1000);
					memset(display_recive_msg.msg_sm, 0, 16);		
					if(msgrcv(display_recive_qid,(void *)&display_recive_msg,recv_length,(long)0,0) < 0)
					{
						printf("display_recive message error\n");
						printf("%s\n", strerror(errno));
						exit(1);
					}				
					display_send_msg.msg_sm[0] = 0xff000000 + (recv_client_data_sm>>24)&0xff;
					display_send_msg.msg_sm[1] = (recv_client_data_sm>>16&0xff);
					//Screen = display_send_msg.msg_sm[1];
					//NewScree();
					if((msgsnd(display_send_qid,&display_send_msg,send_length,0)) < 0)
					{
						printf("display_send message posted\n");
						exit(1);
					}
					printf("display_send_msg0:%x\n",display_send_msg.msg_sm[0]);
					printf("display_send_msg1:%x\n",display_send_msg.msg_sm[1]);	
					recv_client_data_sm = 0;			
					break;
			case	0x3b:								//		

					break;
			case	0x3c:								//

					break;
			case	0x3d:								//
					
					break;
			case	0x3e:								//
					
					break;					
			case	0x3f:								//
					
					break;	


			case	0x4a:								//视频播放预留
				//	lcd_draw_jpg(0,0,"./video.jpg");	
					system("killall -STOP madplay &");	//关音乐
					memset(video_cmd,0,sizeof(video_cmd));
					if(video_head == video_list)
					{
						video_head = video_head->Pprev;
					}					
					sprintf(video_cmd,"%s/%s",video_path,video_head->data);
					mplayer_start(video_cmd);	
					show_cartoon();									
					lcd_draw_jpg(0,0,"./video.jpg");
					break;
			case	0x4b:								//播放		
					show_cartoon();	
				//	lcd_draw_jpg(0,0,"./video.jpg");
					mplayer_pause();
					lcd_draw_jpg(0,0,"./video.jpg");
					break;
			case	0x4c:								//暂停
					mplayer_pause();
					show_cartoon();	
				//	lcd_draw_jpg(0,0,"./video.jpg");	
					mplayer_pause();
					mplayer_pause();	
					lcd_draw_jpg(0,0,"./video.jpg");
					break;
			case	0x4d:								//返回退出	
				//	system("killall -SIGKILL mplayer");
					mplayer_quit();	
					show_cartoon();	
					Screen = (SM>>16&0xff);				//保存新的画面编号
					NewScree();							//显示新的画面		
					lcd_draw_jpg(0,0,"./background.jpg");		
					//mplayer_quit();					
					break;
			case	0x4e:								//快进
					mplayer_seek(10);
					break;					
			case	0x4f:								//回退
					mplayer_seek(-10);					
					break;
			case	0x41:								//上一个			
					mplayer_quit();	
				//	system("killall -SIGKILL madplay");
					show_cartoon();	
					lcd_draw_jpg(0,0,"./video.jpg");
					video_head = video_head->Pprev;	
					if(video_head == video_list)
					{
						video_head = video_head->Pprev;
					}						
					memset(video_cmd,0,sizeof(video_cmd));
					sprintf(video_cmd,"%s/%s",video_path,video_head->data);
					mplayer_start(video_cmd);
					break;
			case	0x42:								//下一个
					mplayer_quit();	
				//	system("killall -SIGKILL madplay");
					show_cartoon();	
					lcd_draw_jpg(0,0,"./video.jpg");
					video_head = video_head->Pnext;	
					if(video_head == video_list)
					{
						video_head = video_head->Pnext;
					}						
					memset(video_cmd,0,sizeof(video_cmd));
					sprintf(video_cmd,"%s/%s",video_path,video_head->data);
					mplayer_start(video_cmd);				
					break;

			case	0x6a:								//相机界面预留
					show_cartoon();
					Screen = (SM>>16&0xff);				//保存新的画面编号					
					camera_init(); 						// 初始化摄像头参数
					camera_start(); 					// 启动摄像头	
					pthread_create(&my_get_touch_tid, NULL, MyGetTouch, NULL);				
					printf("there is pthread_get_touch\n");	
					lcd_draw_jpg(639,0,"./camera.jpg");				
					while(camera_show_flag == 0)
					{								
						camera_show(0,0);
					}				
					if(my_get_touch_return == 2)		//返回
					{
						printf("there is my_get_touch_return\n");
						//lcd_draw_jpg(0,0,"./background.jpg");	
						Screen = 1;
						my_get_touch_return = 0xff;
						camera_close(); 					// 关闭摄像头	
					}
					if(my_get_touch_return == 1)		//拍照
					{
						// 如果文件不存在就创建并打开	
						memset(camera_jpg_cmd,0,sizeof(camera_jpg_cmd));
						sprintf(camera_jpg_cmd,"%s/camera%d.jpg",cemera_photo_path,camera_photo_count);
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
						// 关闭图片文件
						close(tmp_fd);
						memset(camera_jpg_name,0,sizeof(camera_jpg_name));
						sprintf(camera_jpg_name,"camera%d.jpg",camera_photo_count++);
						P_DOUBLE_NODE camera_new = new_node(camera_jpg_name);	//创建新节点		
						add_node(camera_new,photo_list);						//插入新节点

						camera_close(); 					// 关闭摄像头	

						lcd_draw_jpg(0,0,camera_jpg_cmd);
						lcd_draw_jpg(639,0,"./camera.jpg");	

						usleep(1000*1500);
	
						Screen = 1;
						my_get_touch_return = 0x04;
					}						
					if(my_get_touch_return == 0)		//相册
					{
						printf("there is my_get_touch_return\n");
						lcd_draw_jpg(0,0,"./photo.jpg");	
					//	usleep(1000*1000);
						lcd_draw_jpg(0,0,camera_jpg_cmd);
						Screen = 7;
						my_get_touch_return = 0xff;
						camera_close(); 					// 关闭摄像头	
					}					
					//ts_close();
					memset(display_recive_msg.msg_sm, 0, 16);		
					if(msgrcv(display_recive_qid,(void *)&display_recive_msg,recv_length,(long)0,0) < 0)
					{
						printf("display_recive message error\n");
						printf("%s\n", strerror(errno));
						exit(1);
					}						
					/*进程切换，发送触摸值*/
					display_send_msg.msg_sm[0] = 0xff000000 + my_get_touch_return;
					display_send_msg.msg_sm[1] = Screen;
					if((msgsnd(display_send_qid,&display_send_msg,send_length,0)) < 0)
					{
						printf("display_send message posted\n");
						exit(1);
					}
					printf("display_send_msg0:%x\n",display_send_msg.msg_sm[0]);
					printf("display_send_msg1:%x\n",display_send_msg.msg_sm[1]);						
					break;
			case	0x6b:								//拍照
					
					break;
			case	0x6c:								//
	
					break;
			case	0x6d:								//显示相机画面
				
					break;
			case	0x6e:								//
					
					break;					
			case	0x6f:								//
					
					break;


			case	0x7a:								//其他预留
			
					break;
			case	0x7b:								//		

					break;
			case	0x7c:								//

					break;
			case	0x7d:								//
					
					break;
			case	0x7e:								//
					
					break;					
			case	0x7f:								//
					
					break;


			case	0x55:								//查询触摸屏
					// 打开触摸屏
					//ts_open();
					//open_fb0();
					while(wait_load_flag)				//等待加载
					{
						;
					}
					err = ts_getxy(&X, &Y);
					if(err == 1)
					{
						err = TouchKey(X,Y);
						display_send_msg.msg_sm[0] = 0xff000000 + err;
						display_send_msg.msg_sm[1] = Screen;
						if((msgsnd(display_send_qid,&display_send_msg,send_length,0)) < 0)
						{
							printf("display_send message posted\n");
							exit(1);
						}
						printf("display_send_msg0:%x\n",display_send_msg.msg_sm[0]);
						printf("display_send_msg1 Screen:%x\n",display_send_msg.msg_sm[1]);
						err = 0;
					}
					else
					{
						display_send_msg.msg_sm[0] = 0xff00005a;
						if((msgsnd(display_send_qid,&display_send_msg,send_length,0)) < 0)
						{
							printf("display_send message posted\n");
							exit(1);
						}		
						printf("display_send_msg0:%x\n",display_send_msg.msg_sm[0]);				
					}
					// 关闭触摸屏
					//ts_close();
					//close_fb0();
					break;
			default:									//未定义，不处理
				break;
		}
			
		
	}
}



/*
------------------------------------------------------------------------------------------------------------------------
*                                                    屏幕刷新函数的实现
------------------------------------------------------------------------------------------------------------------------
*/
void NewScree(void)
{
	//int8_t i;
	// 打开触摸屏
	//ts_open();
	//clean_fb0();
	switch(Screen)
	{
		case	0x00:				//封面
				lcd_draw_jpg(0,0,"./cover.jpg");	
				break;
		case	0x01:				//操作选择画面
				lcd_draw_jpg(0,0,"./background.jpg");	
				break;	
		case	0x02:				//相册画面
				lcd_draw_jpg(0,0,"./photo.jpg");
				break;		
		case	0x03:				//音频画面	
				lcd_draw_jpg(0,0,"./music.jpg");	
				break;
		case	0x04:				//录音画面
				lcd_draw_jpg(0,0,"./recorder.jpg");	
				break;	
		case	0x05:				//视频画面
				lcd_draw_jpg(0,0,"./video.jpg");	
				break; 
		case	0x06:				//相机画面
				//lcd_draw_jpg(0,0,"./camera.jpg");		
				break; 	
		case	0x07:				//相机相册画面
				lcd_draw_jpg(0,0,"./photo.jpg");	
				break; 								
		default	:  
				break; 
	}
	// 关闭触摸屏
	//ts_close();
	//close_fb0();	
} 
/*
------------------------------------------------------------------------------------------------------------------------
*                                                    触摸屏幕返回键值
------------------------------------------------------------------------------------------------------------------------
*/
int8_t	TouchKey(int16_t x,int16_t y)//判断触摸屏幕代码
{
	int8_t i;
	int16_t (*p)[4];
	if(x < 0 || y < 0 || x > 799 || y > 479)
		return 0x5a;
	switch(Screen)
	{
		case 0:
			p = ScreenKey0;//封面
			break;
		case 1:
			p = ScreenKey1;//操作选择
			break;
		case 2:
			p = ScreenKey2;//相册
			break;
		case 3:
			p = ScreenKey3;//音乐
			break;
		case 4:
			p = ScreenKey4;//录音
			break;
		case 5:
			p = ScreenKey5;//视频
			break;	
		case 6:
			p = ScreenKey6;//相机
			break;
		case 7:
			p = ScreenKey7;//相机
			break;								
		default:
			break;
	}
	i = 0;
	while(1)
	{
		if((x >= p[i][0] && x <= p[i][2]) && (y >= p[i][1] && y <= p[i][3]))
		{
			break;			
		}
		i++;
		if(i == 0x5a)
		{
			break;
		}
	}
	return i;
}

/*
------------------------------------------------------------------------------------------------------------------------
*                                                    后台获取触摸坐标线程
------------------------------------------------------------------------------------------------------------------------
*/
void *MyGetTouch(void *arg)
{
	int x, y;
	int i = 100;
	camera_show_flag = 0;
	my_get_touch_return = 0x5a;
	// 获取点击的坐标
	while(1)
	{
		// 获取点击的坐标
		x = y = 0;
		my_ts_getxy(&x, &y);
		printf("x=%d y=%d\n", x, y);
		printf("Screen:%d\n",Screen);
		my_get_touch_return = TouchKey(x,y);
		printf("my_get_touch_return:%d\n",my_get_touch_return);	
		printf("camera_show_flag:%d\n",camera_show_flag);				
		if(my_get_touch_return == 2)//返回
		{
			camera_show_flag = 1;	
			pthread_cleanup_push(pthread_clean_up,"MyGetTouch");
			pthread_exit((void *)2);
			pthread_cleanup_pop(0);
		}
		if(my_get_touch_return == 0)//相册
		{
			camera_show_flag = 1;	
			pthread_cleanup_push(pthread_clean_up,"MyGetTouch");
			pthread_exit((void *)2);
			pthread_cleanup_pop(0);
		}		
		if(my_get_touch_return == 1)//拍照
		{
			camera_show_flag = 1;	
			pthread_cleanup_push(pthread_clean_up,"MyGetTouch");
			pthread_exit((void *)2);
			pthread_cleanup_pop(0);
		}		
	}
}

/*
------------------------------------------------------------------------------------------------------------------------
*                                                  后台获取客户端连接状态线程
------------------------------------------------------------------------------------------------------------------------
*/
void *recv_client_data(void *arg)
{
	char rec_rslt[1024] = {0};
	
	while(1)
	{

	/*	// 接收客户端发过来的消息
		socket_recv_ack(rec_rslt, sizeof(rec_rslt));
		
		if(strstr(rec_rslt, "打开相册"))
		{
			//printf("rec_rslt = %s\n", rec_rslt);
			recv_client_data_sm = 0X01<<24;			//“显示相册”命令码
			recv_client_data_sm |= 0X02<<16;			//进入“相册”画面,更新屏幕编号	
			printf("recv_client_data_sm = %x\n", recv_client_data_sm);	
			recv_client_data_flag = 1;		
		}
		else if(strstr(rec_rslt, "打开音乐"))
		{
			printf("rec_rslt = %s\n", rec_rslt);
			recv_client_data_flag = 1;
		}
		else if(strstr(rec_rslt, "播放音乐"))
		{
			printf("rec_rslt = %s\n", rec_rslt);
			recv_client_data_flag = 1;
		}
		else if(strstr(rec_rslt, "关闭音乐"))
		{
			printf("rec_rslt = %s\n", rec_rslt);
			recv_client_data_flag = 1;
		}						
		else if(strstr(rec_rslt, "打开视频"))
		{
			printf("rec_rslt = %s\n", rec_rslt);
			recv_client_data_flag = 1;
		}				
		else if(strstr(rec_rslt, "打开相机"))
		{
			printf("rec_rslt = %s\n", rec_rslt);
			recv_client_data_flag = 1;
		}
		else
		{
			printf("无效输入\n");
			recv_client_data_flag = 1;
		}
		*/			
	}
}

/*
------------------------------------------------------------------------------------------------------------------------
*                                                    工具函数
------------------------------------------------------------------------------------------------------------------------
*/
void pthread_clean_up(void *arg)
{
	printf("cleanup: %s\n",(char *)arg);
}

void show_cartoon()//过场动画
{
	wait_load_flag = 1;
	lcd_draw_jpg(0,0,"buffering1.jpg");
	usleep(1000*10);
	lcd_draw_jpg(0,0,"buffering2.jpg");
	usleep(1000*10);
	lcd_draw_jpg(0,0,"buffering3.jpg");
	usleep(1000*10);
	lcd_draw_jpg(0,0,"buffering4.jpg");
	usleep(1000*10);
	lcd_draw_jpg(0,0,"buffering1.jpg");
	wait_load_flag = 0;
}

//初始化链表
P_DOUBLE_NODE list_init(char *new_data)
{
	P_DOUBLE_NODE head = (P_DOUBLE_NODE)malloc(sizeof(DOUBLE_NODE));
	if(head == NULL)
	{
		printf("error of malloc!\n");
        exit(1);
	}
	head->data = new_data;
	head->Pnext = head;
	head->Pprev = head;
	return head;
} 

//创建新节点
P_DOUBLE_NODE new_node(char *new_data)
{
	P_DOUBLE_NODE new = (P_DOUBLE_NODE)malloc(sizeof(DOUBLE_NODE));
	if(new == NULL)
	{
		printf("error of malloc!\n");
        exit(1);
	}	
	new->data = new_data;
	new->Pnext = NULL;
	new->Pprev = NULL;
	return new;
}

//加入新节点
void add_node(P_DOUBLE_NODE new,P_DOUBLE_NODE list)
{
	P_DOUBLE_NODE p = list;
	while(p->Pnext != list)
	{
		p = p->Pnext;
	}
	new->Pprev = list->Pprev;
	new->Pnext = list;
	list->Pprev = new;
	new->Pprev->Pnext = new;
}

