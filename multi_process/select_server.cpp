#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>

int main()
{
    int listenfd;
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    printf("listenfd: %d\n", listenfd);
    if(listen < 0) {
        perror("socket");
        exit(-1);
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5005);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret < 0) {
        perror("bind");
        exit(-1);
    }

    ret = listen(listenfd, 5);
    if(ret < 0){
        perror("listen");
        exit(-1);
    }

    fd_set allset, readset;
    FD_ZERO(&allset);
    FD_ZERO(&readset);
    FD_SET(listenfd, &allset);
    int maxfd = listenfd;

    while(1){
        readset = allset;
        int res = select(maxfd + 1, &readset, NULL, NULL, NULL);
        if(res < 0){
            perror("select");
            exit(-1);
        } else if(res == 0){
            continue;
        } else {
            if(FD_ISSET(listenfd, &readset)){
                struct sockaddr_in clientaddr; 
                int len = sizeof(clientaddr);
                int connfd = accept(listenfd, (struct sockaddr*)&clientaddr, (socklen_t*)&len);
                printf("connfd: %d\n", connfd);
                FD_SET(connfd, &allset);
                maxfd = maxfd > connfd ? maxfd : connfd;
            }

            for(int i = listenfd + 1; i <= maxfd; i++) {
                if(FD_ISSET(i, &readset)){
                    char buf[1024];
                    int retlen = read(i, &buf, sizeof(buf));
                    if(retlen < 0) {
                        perror("read");
                    } else if(retlen == 0){
                        printf("connect closed.");
                        close(i);
                        FD_CLR(i, &allset);
                    } else {
                        printf("read buf = %s\n", buf);
                        write(i, buf, strlen(buf) + 1);
                    }
                }
            }
        }
    }
    close(listenfd);
    return 0;
}