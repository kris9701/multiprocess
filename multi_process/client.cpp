#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_SIZE_INFO 1024

int main(int argc, char* argv[])
{
    if(argc <= 1) {
        printf("Enter port number!");
        exit(-1);
    }

    //  Create and judge socket.
    int connfd = socket(PF_INET, SOCK_STREAM, 0);
    if(connfd < 0) {
        perror("socket");
        exit(-1);
    }

    // Connect with server.
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = PF_INET;
    inet_pton(AF_INET, "192.168.1.107", &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(atoi(argv[1]));
    int ret = connect(connfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret < 0) {
        perror("connect");
        exit(-1);
    }

    char buf[MAX_SIZE_INFO];
    int i = 0;
    // Communicate with server.
    while(1){
        sprintf(buf, "data id:%d\n", i++);
        write(connfd, &buf,strlen(buf) + 1);

        int dataLen = read(connfd, buf, sizeof(buf));
        if(dataLen == -1) {
            perror("read");
            exit(-1);
        } else if(dataLen > 0) {
            printf("recv server : %s\n", buf);
        } else if(dataLen == 0) {
            printf("server closed.");
            break;
        }
        sleep(1);
    }

    close(connfd);
    return 0;
}