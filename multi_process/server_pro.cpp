#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <wait.h>
#define MAX_SIZE_INFO 1024

void recycleChild(int arg)
{
    // int ret;
    while(1) {
        int ret = waitpid(-1, NULL, WNOHANG);
        if(ret > 0) {
            printf("child process %d has been recycle.\n", ret);
        } else {
            break;
        }
    }
    
}

int main(int argc, char* argv[])
{
    // Avoid zombie process.Recycle the resources of child processes.
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = recycleChild;
    sigemptyset(&act.sa_mask);
    // Register to catch the signal.
    sigaction(SIGCHLD, &act, NULL);

    if(argc <= 1) {
        printf("Enter port number!");
        exit(-1);
    }

    //  Create and judge socket.
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(listenfd < 0) {
        perror("socket");
        exit(-1);
    }

    // Bind address.
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons( atoi(argv[1]) );
    servaddr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret != 0 ) {
        perror("bind");
        close(listenfd);
        exit(-1);
    }

    // Set to listen.
    ret = listen(listenfd, 5);
    if(ret != 0) {
        perror("listen");
        exit(-1);
    }

    // Accept connection.
    while(1) {

        struct sockaddr_in clientaddr;
        int len = sizeof(clientaddr);
        int commfd = accept(listenfd, (struct sockaddr*)&clientaddr, (socklen_t*)&len);
        if(ret == -1){
            if(errno == EINTR) {
                continue;
            }
            perror("accept");
            exit(-1);
        }

        // Create child process to communicate with every client.
        pid_t pid = fork();
        if(pid == 0) {
            // child process.
            char childIp[16];
            inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, childIp, sizeof(childIp));

            // communicate with child process.

            // print child process message.
            unsigned short childPort;
            childPort = ntohs(clientaddr.sin_port);
            printf("%s is commicating, port is %d\n", childIp, childPort);

            // Recv message.
            char buf[MAX_SIZE_INFO];
            while(1) {
                if(commfd > 0){
                    int dataLen = read(commfd, &buf, sizeof(buf));
                    if(dataLen < 0) {
                        perror("read");
                        exit(-1);
                    } else if(dataLen > 0) {
                        printf("Recv data from %s:%s\n", childIp, buf);
                    } else {
                        printf("Communicate of %s closed.\n", childIp);
                        break;
                    }
                    
                    // reply to client. 
                    write(commfd, &buf, strlen(buf) + 1);
                }
            }
            close(commfd);
            exit(0);
        }
    }
    close(listenfd);
    return 0;
}