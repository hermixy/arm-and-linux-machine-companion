#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

int recv_filesize = -1;
int sockfd = -1;

int client_link_server()
{
	int ret = 0;
	
	
	sockfd = socket(AF_INET,SOCK_STREAM,0); //创建套接字
	
	struct sockaddr_in dest_addr; // 定义本机IP地址和端口结构体
	dest_addr.sin_family = AF_INET; //地址族
	dest_addr.sin_port = htons(8888); //端口号
	dest_addr.sin_addr.s_addr = inet_addr("192.168.0.10"); // 目标是开发板的ip地址
	// 客户端连接指定的服务器的IP地址和端口
	int status = connect(sockfd, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
	
	return status;

}

void client_recvfile()
{
	int pcm_size = 0;
	// 接收文件大小值
	read(sockfd, &recv_filesize, sizeof(recv_filesize)); // 接收数据
	// printf("IP：%s  port：%d : %d\n", inet_ntoa(dest_addr.sin_addr), ntohs(dest_addr.sin_port), recv_filesize);
	//printf("收到了文件大小\n");
	
	// 打开或者创建文件
	int pcm_fd = open("wav/yysb.pcm", O_RDWR|O_CREAT, 0777); // 打开文件
	
	char recv_buf[1024] = {0};
	int ret = -1;
	while(1)
	{
		// 循环接收文件
		memset(recv_buf, 0, sizeof(recv_buf));
		ret = read(sockfd, recv_buf, sizeof(recv_buf)); // 接收数据
		if(ret <= 0) // 服务器退出了
		{
			printf("服务器退出了\n");
			exit(0);
		}
			
		printf("ret = %d\n", ret);
		write(pcm_fd, recv_buf, ret);
		
		pcm_size += ret;
		
		if(pcm_size >= recv_filesize) // 累计读取的数据大于或者等于文件实际的数据
			break;
	}
	printf("音频文件接收完成\n");
	close(pcm_fd);
}

// 发送指定数据
void send_data(char *buf)
{
	write(sockfd, buf, strlen(buf)); // 发送数据
}

void client_close()
{
	close(sockfd);
}
