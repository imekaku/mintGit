//客户端，服务器回射程序
//加入头部协议
//2015年11月28日15:39:30
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

//定义头部协议
struct packet
{
    int len;//发送数据的长度
    char buf[1024];//发送的数据
};

//重定义读取函数
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
               return count - nleft;//返回已经读取的字节数-剩余的字节数
               //表示读取的字节数小于要读取的字节数

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
            if(errno == EINTR)//连接中断的情况
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

void do_server(int conn)
{
    struct packet recvbuf;
    int n;
	while(1)
	{
		memset(&recvbuf, 0, sizeof(recvbuf));
        //向套接字中拿出4个字节放在recvbuf.len中
		int ret = readn(conn, &recvbuf.len, 4);
        if(ret == -1)
            ERR_EXIT("read");
        else if(ret < 4)//如果读取的字节数小于要读取的字节数，表示对方关闭了
        {
            printf("client close\n");
            break;
        }

        n = ntohl(recvbuf.len);
        ret = readn(conn, recvbuf.buf, n);
        if(ret == -1)
            ERR_EXIT("read");
        else if(ret < n)
        {
            printf("client close\n");
            break;
        }

        //输出内容
		fputs(recvbuf.buf, stdout);
        //向已连接套接字中写入
		write(conn, &recvbuf, 4+n);
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
