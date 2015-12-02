//客户端，服务器回射程序
//服务器端已使用多线程，使得可以连接多个客户端
//2015年11月28日14:25:36
//client.c
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE);\
	}while(0)

int main(void)
{
	int sock;
    //创建一个套接字
	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
		ERR_EXIT("socket");
    //初始化ip地址
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	//servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //连接服务器
	if(connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
		ERR_EXIT("connect");
    printf("serverIP=%s\n", inet_ntoa(servaddr.sin_addr));

	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};
    //获取内容
	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{
        //向套接字中写入，读取
		write(sock, sendbuf, strlen(sendbuf));
		read(sock, recvbuf, sizeof(recvbuf));
		
        //输出
		fputs(recvbuf, stdout);
        //清空
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}

	close(sock);

	return 0;
}
