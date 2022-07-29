#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define MAX_SIZE_INFO 1024

struct sockInfo{
    int fd;
    pthread_t tid;
    struct sockaddr_in addr;
};
struct sockInfo sockinfos[128]; 

void * working(void * arg) {
    struct sockInfo * pinfo = (struct sockInfo *)arg;

    char cliIp[16];
    inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, cliIp, sizeof(cliIp));
    unsigned short cliPort = ntohs(pinfo->addr.sin_port);
    printf("client ip is : %s, prot is %d\n", cliIp, cliPort);

    char recvBuf[MAX_SIZE_INFO];
    while(1) {
        int len = read(pinfo->fd, &recvBuf, sizeof(recvBuf));

        if(len == -1) {
            perror("read");
            exit(-1);
        }else if(len > 0) {
            printf("recv client : %s\n", recvBuf);
        } else if(len == 0) {
            printf("client closed....\n");
            break;
        }
        write(pinfo->fd, recvBuf, strlen(recvBuf) + 1);
    }
    close(pinfo->fd);
    return NULL;
}

int main(int argc, char* argv[])
{
    int listenfd;
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(listenfd < 0){
        perror("listenfd");
        exit(-1);
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[1]));
    servaddr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret < 0) {
        perror("bind");
        exit(-1);
    }

    ret = listen(listenfd, 5);
    if(ret<0){
        perror("listen");
        exit(-1);
    }
    
    int max = sizeof(sockinfos)/sizeof(sockinfos[0]);
    for(int i=0;i<max;i++) {
        bzero(&sockinfos[i], sizeof(sockinfos[i]));
        sockinfos[i].fd = -1;
        sockinfos[i].tid = -1;
    }


    while(1) {

        struct sockaddr_in connaddr;
        int len = sizeof(connaddr);
        int clientfd = accept(listenfd, (struct sockaddr*)&connaddr, (socklen_t*)&len);

        struct sockInfo *pinfo;
        for(int j = 0; j < max; j++){
            if(sockinfos[j].fd == -1){
                pinfo = &sockinfos[j];
                break;
            }

            // When there are no idle threads, wait.
            if(j == max - 1) {
                sleep(1);
                j = 0;
            }
        }

        pinfo->fd = clientfd;
        memcpy(&pinfo->addr, &connaddr, sizeof(connaddr));
        pthread_create(&pinfo->tid, NULL, working, pinfo);
        pthread_detach(pinfo->tid);

    }

    close(listenfd);
    return 0;
}