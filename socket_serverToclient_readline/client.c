//2015年11月29日20:49:40
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

struct packet
{
    int len;
    char buf[1024];
};

ssize_t readn(int fd, void *buf, size_t count)
{
    size_t nleft = count;//nleft剩余的字节数
    ssize_t nread;
    char *bufp = (char *)buf;

    while(nleft > 0)
    {
        if((nread = read(fd, bufp, nleft)) < 0)
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
        else if(nread == 0)
            return count - nleft;
        bufp = bufp + nread;
        nleft = nleft - nread;
    }
    return count;
}

ssize_t writen(int fd, const void *buf, size_t count)
{
    size_t nleft = count;
    ssize_t nwritten;
    char *bufp = (char *)buf;

    while(nleft > 0)
    {
        if((nwritten = write(fd, bufp, nleft))<0)
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
        else if(nwritten == 0)
            continue;
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

    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    if(getsockname(sock, (struct sockaddr*)&localaddr, &addrlen)<0)
        //其中sock是已连接的套接口
        ERR_EXIT("getsockname");

    printf("localIP=%s, localPort=%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));

	char sendbuf[1024] = {0};
	char recvbuf[1024] = {0};
    //获取内容
    int n;
	while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
	{
		writen(sock, sendbuf, strlen(sendbuf));
        int ret = readline(sock, recvbuf, sizeof(recvbuf));
        
        if(ret == -1)
            ERR_EXIT("read");
        else if(ret == 0)
        {
            printf("client close\n");
            break;
        }

        //输出
		fputs(recvbuf, stdout);
        //清空
		memset(sendbuf, 0, sizeof(sendbuf));
		memset(recvbuf, 0, sizeof(recvbuf));
	}

	close(sock);

	return 0;
}
