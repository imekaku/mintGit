//客户端，服务器回射程序
//已启动多线程，使得服务器可以连接多个客户端
//2015年11月28日14:24:40
//server.c
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

void do_server(int conn)
{
    char recvbuf[1024];
	while(1)
	{
		memset(recvbuf, 0, sizeof(recvbuf));
        //从连接套接字中拿出内容
		int ret = read(conn, recvbuf, sizeof(recvbuf));
        //如果内容为0，跳出循环
        if(ret == 0)
        {
            printf("client close\n");
            break;
        }
        else if(ret == -1)
            ERR_EXIT("read");
        //输出内容
		fputs(recvbuf, stdout);
        //向已连接套接字中写入
		write(conn, recvbuf, ret);
	}
}
int main(void)
{
	int listenfd;
    //设置一个监听套接字
	if((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
		ERR_EXIT("socket");
    //初始化服务器地址
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(5188);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //设置在TIME_WAIT消失前可连接
    int on = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))<0)
        ;

    //绑定监听套接字和服务器地址
	if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
		ERR_EXIT("bind");
    //
	if(listen(listenfd,SOMAXCONN)<0)
		ERR_EXIT("listen");

    //设置对等方套接字地址
	struct sockaddr_in peeraddr;
	socklen_t peerlen = sizeof(peeraddr);

    int conn;//已连接套接字
    pid_t pid;
    while(1)
    {
        //从完成三次握手的队列中拿出已连接套接字，放在conn中
	    if((conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen))<0)
		    ERR_EXIT("accept");

        // 打印出连接的客户端的地址和端口号
        printf("ip=%s, port=%d\n",inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

        //多线程，子线程用于连接，父线程用于继续从完成三次握手的队列中拿出套接字
        pid = fork();
        if(pid == -1)
            ERR_EXIT("fork");
        if(pid == 0)
        {
            //关闭监听套接字
            close(listenfd);
            do_server(conn);
            exit(EXIT_SUCCESS);
        }
        else
            //关闭已连接套接字
            close(conn);
    } 

	return 0;
}
