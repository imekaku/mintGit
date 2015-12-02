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

    struct packet sendbuf;
    struct packet recvbuf;
    memset(&sendbuf, 0, sizeof(sendbuf));
    memset(&recvbuf, 0, sizeof(recvbuf));
	//char sendbuf[1024] = {0};
	//char recvbuf[1024] = {0};
    //获取内容
    int n;
	while(fgets(sendbuf.buf, sizeof(sendbuf.buf), stdin) != NULL)
	{
        n = strlen(sendbuf.buf);
        sendbuf.len = htonl(n);
        //向套接字中写入，读取
		writen(sock, &sendbuf, 4+n);

        int ret = readn(sock, &recvbuf.len, 4);
        if(ret == -1)
            ERR_EXIT("read");
        else if(ret < 4)
        {
            printf("client close\n");
            break;
        }

        n = ntohl(recvbuf.len);
        ret = readn(sock, recvbuf.buf, n);
        if(ret == -1)
            ERR_EXIT("read");
        else if(ret < n)
        {
            printf("client close\n");
            break;
        }


        //输出
		fputs(recvbuf.buf, stdout);
        //清空
		memset(&sendbuf, 0, sizeof(sendbuf));
		memset(&recvbuf, 0, sizeof(recvbuf));
	}

	close(sock);

	return 0;
}
