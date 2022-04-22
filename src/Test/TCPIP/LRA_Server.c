/**
 * @file LRA_Server.c
 * @author your name (you@domain.com)
 * @brief test server
 * @ref https://github.com/davidleitw/socket/blob/master/tcp_example/tcp_server.c
 * @ref https://www.itread01.com/content/1549522442.html -- socket tune 
 * @ref https://github.com/IronsDu/brynet -- maybe use this to write version 2
 * @ref https://blog.csdn.net/lileiyuyanqin/article/details/79665896 -- nonblock
 * @version 0.1
 * @date 2022-04-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "LRA_TCPIP.h"

// global var
char tx_buf[MAX_BUF_BYTE] = {0};
char rx_buf[MAX_BUF_BYTE] = {0};

float timestamp, x, y, z;
int checksum;
const int serverPort = 8787;

const int backlog = 5;

void* clientSocket(void* param)
{

    struct client_info * info = param;

    strcpy(tx_buf, "12.000,0.5,0.32,1324.023,777");
    send(info->sockfd, tx_buf, sizeof(tx_buf), 0);
    memset(tx_buf, 0, sizeof(tx_buf));

    while (recv(info->sockfd, rx_buf, sizeof(rx_buf), 0)) {
        
        // break check
        if (strcmp(rx_buf, "sendAD") == 0) {
            memset(rx_buf, 0, sizeof(rx_buf));
            break;
        }

        // parse recv data

        // show what we got
        printf("get message from [%s:%d]: ",
                inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
        printf("%s\n", rx_buf);

        // send new data
        strcpy(tx_buf, "12.000,15.5,0.32,1324.023,777\0");

        // senback to client
        if (send(info->sockfd, tx_buf, sizeof(tx_buf), 0) < 0) {
            printf("send data to %s:%d, failed!\n", 
                    inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
            memset(tx_buf, 0, sizeof(tx_buf));
            break;
        }

        // 清空 message buffer
        memset(rx_buf, 0, sizeof(rx_buf));
    }

    // 關閉 reply socket，並檢查是否關閉成功
    if (close(info->sockfd) < 0) {
        perror("close socket failed!");
    }else{
        printf("Socket closed from %s:%d success!\n", 
            inet_ntoa(info->clientAddr.sin_addr), ntohs(info->clientAddr.sin_port));
    }

}

int main()
{
    int client_idx = 0;
    pthread_t clients[backlog];

    // create socket
    int socket_fd = socket(PF_INET , SOCK_STREAM , 0);
    if (socket_fd < 0){
        printf("Fail to create a socket.");
    }

    // server info 
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(serverPort)
    };

    //bind socket to certain port 
        if (bind(socket_fd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind socket failed!");
        close(socket_fd);
        exit(0);
    }

    // listen to port, setting BACKLOG
    if (listen(socket_fd, backlog) == -1) {
        printf("socket %d listen failed!\n", socket_fd);
        close(socket_fd);
        exit(0);
    }

    printf("server [%s:%d] --- ready\n", 
    inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    // main loop
    while(1)
    {
        int reply_sockfd;
        struct sockaddr_in clientAddr;
        int client_len = sizeof(clientAddr);

        // 從 complete connection queue 中取出已連線的 socket
        // 之後用 reply_sockfd 與 client 溝通
        reply_sockfd = accept(socket_fd, (struct sockaddr *)&clientAddr, &client_len);
        printf("Accept connect request from [%s:%d]\n", 
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // 建立 thread 與 thread 的參數
        struct client_info *newClientinfo = malloc(sizeof(struct client_info));
        memcpy(&newClientinfo->sockfd, &reply_sockfd, sizeof(int));
        memcpy(&newClientinfo->clientAddr, &clientAddr, sizeof(struct sockaddr_in));

        if (pthread_create(&clients[client_idx++], NULL, clientSocket, newClientinfo) != 0){
            printf("Failed to create new Thread!");
        }

    }

        // 關閉 socket，並檢查是否關閉成功
        if (close(socket_fd) < 0) {
            perror("close socket failed!");
        }
        return 0;

}

