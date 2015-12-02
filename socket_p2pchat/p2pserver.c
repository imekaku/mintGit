//点对点聊天程序
//已实现多线程实现读取和写入的操作
//已实现使用信号量，使得当客户端或者服务器关闭时，对方关闭
//p2pserver.c
//2015年11月27日11:24:37
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#define ERR_EXIT(m) \
	do \
	{ \
		perror(m); \
		exit(EXIT_FAILURE);\
	}while(0)

void hadler(int sig)
{
    printf("recv a sig = %d\n", sig);
    exit(EXIT_SUCCESS);
}

int main(void)
{
    //定义一个监听套接字
	int listenfd;
	if((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
		ERR_EXIT("socket");
    //初始化服务器地址
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //设置time_wait状态
    int on = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))<0)
        ;

    //绑定监听套接字到服务器地址
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
		ERR_EXIT("bind");
	if(listen(listenfd,SOMAXCONN)<0)
		ERR_EXIT("listen");
    
    //定义一个对等地址
	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);

    //主动套接字
    int conn;
    //从完成三次握手的队列中提取出套接字，成为主动套接字
    if((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen))<0)
        ERR_EXIT("accept");

    pid_t pid;
    pid = fork();
    if(pid == -1)
        ERR_EXIT("fork");
    if(pid == 0)
    {
        signal(SIGUSR1, hadler);
        char sendbuf[1024];
        while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
        {
            write(conn, sendbuf, strlen(sendbuf));
            memset(sendbuf, 0, sizeof(sendbuf));
        }
        exit(EXIT_SUCCESS);
    }
    else
    {
        char recvbuf[1024];
        while(1)
        {
            memset(recvbuf, 0, sizeof(recvbuf));
            int ret = read(conn, recvbuf, sizeof(recvbuf));
            if(ret == -1)
                ERR_EXIT("read");
            if(ret == 0)
            {
                printf("peer close\n");
                break;
            }
            else
                fputs(recvbuf, stdout);	
        }
        exit(EXIT_SUCCESS);
    }

	close(conn);
	close(listenfd);
	return 0;
}
