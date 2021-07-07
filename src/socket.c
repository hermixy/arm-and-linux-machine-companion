
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "socket.h"
#include "lcd.h"

int sockfd = -1;
int new_fd = -1;

int client_status = 0;

// 初始化socket服务器
void socket_server_init(int port)
{
	sockfd = socket(AF_INET,SOCK_STREAM,0); //创建套接字
	
	// if()
	
	struct sockaddr_in my_addr; // 定义本机IP地址和端口结构体
	my_addr.sin_family = AF_INET; //地址族
	my_addr.sin_port = htons(port); //端口号
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY); //自动获取本机IP //inet_addr("192.168.1.20"); // INADDR_ANY
	memset(my_addr.sin_zero,0,sizeof(my_addr.sin_zero));
	int ret = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
	
	listen(sockfd, 5); // 设置监听端口
}

int socket_accept()
{
	struct sockaddr_in their_addr; // 创建空结构体，存放客户端的信息
	// 服务器已经准备好了，等到连接
	int sin_size = sizeof(struct sockaddr_in);
	
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); // 如果有客户端连接了，会创建一个新的文件new_fd和客户端进行连接
	printf("客户: %s 已经连接上服务器！\n", inet_ntoa(their_addr.sin_addr));
	client_status = 1;
	return new_fd;
}

// 发送指定数据
void send_data(char *buf)
{
	if(client_status) // 客户端在线才会发送
		write(new_fd, buf, strlen(buf)); // 发送数据
}

void socket_send_file()
{
	int ret = 0;
	char pcm_buf[1024] = {0};
	int pcm_size = 0;
	// 录音 --> 产生一个音频文件
	system("arecord -d3 -c1 -r16000 -fS16_LE -traw yysb.pcm");
	
	// 获取发送文件的大小，并且发送给接收方
	int file_size = file_size_get("./yysb.pcm");
	write(new_fd, &file_size, sizeof(file_size)); // 发送文件大小
	usleep(1000*100);
	
	// 打开音频文件
	int pcm_fd = open("./yysb.pcm", O_RDWR);
	
	// 读取文件数据
	while(1)
	{
		memset(pcm_buf, 0, sizeof(pcm_buf));
		
		// ret 表示读取到的实际字节数
		ret = read(pcm_fd, pcm_buf, sizeof(pcm_buf)); 
		
		pcm_size += ret;
		
		// 发送文件数据
		write(new_fd, pcm_buf, ret);
		usleep(1000*10);
		
		if(pcm_size >= file_size) // 累计读取的数据大于或者等于文件实际的数据
			break;
		
	}
	
	close(pcm_fd);
}

// 接收Ubuntu返回的结果
void socket_recv_ack(char *rec_rslt, int len)
{
	int ret = read(new_fd, rec_rslt, len); 
	if(ret == 0) // 服务器退出了
	{
		printf("客户端断开退出了\n");
		client_status = 0;
		close(new_fd);
		socket_accept(); // 重新等待客户端连接
	}
}