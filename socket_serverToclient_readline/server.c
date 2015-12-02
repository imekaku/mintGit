//客户端，服务器回射程序
//加入头部协议
//2015年11月28日15:39:30
//server.c
//2015年11月29日20:04:02
//完成readline函数的封装
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
/*
struct packet
{
    int len;
    char buf[1024];
};
*/
ssize_t readn(int fd, void *buf, size_t count)
{
    size_t nleft = count;//nleft剩余的字节数
    ssize_t nread;//接受到的字节数
    char *bufp = (char *)buf;//char指针指向buf

    //如果还有字节就不断循环
    while(nleft > 0)
    {
        //从bufp中读取nleft到fd套接字中
        if((nread = read(fd, bufp, nleft))<0)
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
        //对方关闭
        else if(nread == 0)
               return count - nleft;//全部要读取字节数

        bufp = bufp + nread;
        nleft = nleft - nread;
    }
    return count;
}

ssize_t writen(int fd, const void *buf, size_t count)
{
    size_t nleft = count;//nleft剩余的字节数
    ssize_t nwritten;
    char *bufp = (char *)buf;//char指针指向buf

    //如果还有字节就不断循环
    while(nleft > 0)
    {
        //从bufp中读取nleft到fd套接字中
        if((nwritten = write(fd, bufp,nleft))<0)
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
        //对方关闭
        else if(nwritten == 0)
               continue;//全部要读取字节数

        bufp = bufp + nwritten;
        nleft = nleft - nwritten;
    }
    return count;
}

size_t recv_peek(int sockfd, void *buf, size_t len)
{
    while(1)
    {
        int ret = recv(sockfd, buf, len, MSG_PEEK);
        if(ret == -1 && errno == EINTR)
            continue;
        return ret;
    }
}

size_t readline(int sockfd, void *buf, size_t maxline)
{
    int ret;
    int nread;
    char *bufp = buf;
    int nleft = maxline;
    while(1)
    {
        ret = recv_peek(sockfd, bufp, nleft);//bufp指针所指向的缓冲区中
        if(ret < 0)//表示失败,无需信号中断的判断
            return ret;
        else if(ret == 0)//
            return ret;
        nread = ret;//接收到的字节数
        int i;
        for(i = 0; i < nread; i++)
        {
            if(bufp[i] == '\n')
            {
                ret = readn(sockfd, bufp, i+1);//从缓冲区移除
                if(ret != (i+1))
                    exit(EXIT_FAILURE);//失败
                return ret;
            }
        }
        if(nread > nleft)
            exit(EXIT_FAILURE);
        nleft = nleft - nread;
        ret = readn(sockfd, bufp, nread);
        if(ret != nread)
            exit(EXIT_FAILURE);
        bufp = bufp + nread;
    }

    return -1;//程序出错
}

void do_server(int conn)
{
    char recvbuf[1024];
	while(1)
	{
		memset(&recvbuf, 0, sizeof(recvbuf));
        //从连接套接字中拿出内容
		int ret = readline(conn, recvbuf, 1024);
        //如果内容为0，跳出循环
        if(ret == -1)
            ERR_EXIT("readline");
        if(ret == 0)
        {
            printf("client close\n");
            break;
        }
        //输出内容
		fputs(recvbuf, stdout);
        //向已连接套接字中写入
		write(conn, recvbuf, strlen(recvbuf));
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
