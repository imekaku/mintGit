//点对点聊天程序实现
//实现使用多线程使得写入和读取在不同的进程中
//实现使用信号量，当对方关闭时候，关闭自己
//p2pclient.c
//2015年11月27日11:24:05
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

void handler(int sig)
{
    printf("recv a sig = %d\n", sig);
    exit(EXIT_SUCCESS);
}

int main(void)
{
    //定义一个套接字
	int sock;
	if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
		ERR_EXIT("socket");
    
    //初始化ip地址
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	//servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //连接套接字套服务器IP地址
	if(connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
		ERR_EXIT("connect");
    //打印服务器IP地址
    printf("serverIP=%s\n", inet_ntoa(servaddr.sin_addr));

    //开启两个进程
    pid_t pid;
    pid = fork();
    if(pid == -1)
        ERR_EXIT("fork");
    if(pid == 0)
    {
        char recvbuf[1024];
        while(1)
        {
            memset(recvbuf, 0, sizeof(recvbuf));
            int ret = read(sock, recvbuf, sizeof(recvbuf));
            if(ret == -1)
                ERR_EXIT("read");
            else if (ret == 0)
            {
                printf("peer close\n");
                break;
            }
           fputs(recvbuf, stdout);
        }
        close(sock);
        kill(getppid(), SIGUSR1);
    }
    else
    {
        signal(SIGUSR1,handler);
    	char sendbuf[1024] = {0};
    	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    	{
    		write(sock, sendbuf, strlen(sendbuf));
     		memset(sendbuf, 0, sizeof(sendbuf));
    	}
        close(sock);
    }
	return 0;
}
