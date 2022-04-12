/**
 * @file LRA_Client.c
 * @author your name (you@domain.com)
 * @ref https://github.com/davidleitw/socket/blob/master/tcp_example/tcp_client.c
 * @brief Version 1.0 of TCPIP connection
 * @version 1.0
 * @date 2022-03-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "LRA_TCPIP.h"

// information setting
const char* local_ip = "127.0.0.1";
char* server_ip;

const int NCCU_port = 8787;
const int local_port = 8787;

////////////////////////////////
// some param in main
/* user tune -- connection param*/
const char* NCCU_ip = "";
int server_port = local_port;
/* user tune -- retry param*/
const unsigned int max_retry_num = 5;
const unsigned int sleep_time = 5;
/*user tune -- delim*/
const char* dlm = ",";
////////////////////////////////

// data var
float timestamp = 0.0;
float x = 0.0;
float y = 0.0;
float z = 0.0;
char* checksum;
char tx_buf[MAX_BUF_BYTE] = {0};
char rx_buf[MAX_BUF_BYTE] = {0};

// looping var
int looping = 1;

// functions
/**
 * @brief private communication init cmd 
 * 
 * @return int, success : 1
 */
int communication_init()
{
    // TODO init setting
    return 1;
}

/**
 * @brief try to build connection, including timeout mechanism
 * 
 * @param socket_fd 
 * @param server_info 
 * @param max_retry_num 
 * @param sleep_time 
 * @return int 
 */
int try_connection(int* socket_fd, struct sockaddr_in server_info, unsigned int max_retry_num, unsigned int sleep_time)
{
    int retry_num = 0;
    // try to connect to NCCU server
    while (connect(*socket_fd, (struct sockaddr*)&server_info, sizeof(server_info)) < 0) {

        perror("Connect to socket fd failed");

        if (retry_num > max_retry_num) {
            printf("Max retry exceeded, connection abort\n");
            exit(EXIT_FAILURE);
        }
            
        sleep(sleep_time);
        retry_num++;
    }

    // connection built
    printf("Connect server [%s:%d] success\n", 
            inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));

    // init connection by send something to server
    return communication_init();
}

/**
 * @brief check user input and judge
 * 
 * @return int 
 */
int userinput_check()
{
    // read stdin 
    char user_input[MAX_BUF_BYTE];
    int buf_count = read(STDIN_FILENO, user_input, MAX_BUF_BYTE-1);

    // add null char
    int last = buf_count < MAX_BUF_BYTE ? buf_count + 1 : MAX_BUF_BYTE;
    user_input[last-1] = '\0';  // idx from 0
    
    if (!strcmp(user_input, "q\n") || !strcmp(user_input, "quit\n"))
        return EXIT_PROCESS;

    if (!strcmp(user_input, "d\n") || !strcmp(user_input, "disconnect\n") || !strcmp(user_input, "disc\n"))
        return SEND_DISCONNECT_CMD;

    if (!strcmp(user_input, "r\n") || !strcmp(user_input, "reconnection\n") || !strcmp(user_input, "reconn\n"))
        return SEND_RECONNECT_CMD;
        
    return OK;
}

/**
 * @brief send feedback to server
 * 
 * @param parseable 
 * @param socket_fd 
 * @return int 
 */
int send_back(int parseable, int socket_fd)
{
    // safe check 
    if(socket_fd <= 0)
        return 0;

    switch (parseable)
    {
        /*get wrong format*/
        case UNPARSEABLE:
            // do nothing
            strcpy(tx_buf, "fail2Parse");   // failed to parse rx_buf
            break;

        /*Parse to data successed, send checksum back*/
        case DATA:
        {
            // char tmp[MAX_BUF_BYTE] = {0};
            // FIXME: checksum should be string, does this work?
            // sprintf(tmp,"%d",checksum);
            strcpy(tx_buf, checksum);
        }
            break;

        case PASSIVE_DISCONNECT:
            /*Get disconnect cmd from server, passive*/
            strcpy(tx_buf, "getPD");
            break;

        // ================== only call when we want to abort disconnection ================= //

        case ACTIVE_DISCONNECT:
            /*Send a disconnect cmd to server, active*/
            strcpy(tx_buf, "sendAD");
            break;

        case ACTIVE_RECONNECT:
            /*Send a resconnect cmd to server, active*/
            strcpy(tx_buf, "sendAR");
            break;

        default:
            // do nothing
            break;
    }
    send(socket_fd, tx_buf, strlen(tx_buf), 0);
}

/**
 * @brief parse tx_buf and return tx_buf type
 * 
 * @param tx_buf 
 * @return int 
 */
int parse(char* rx_buf, const char* delim)
{
    int parseable = UNPARSEABLE;
    char* str_array[PARSE_STR_BUF_BYTE] = {0};
    char* token;
    int cur = 0;

    for(token = strtok(rx_buf, delim); 
        token != NULL && cur < PARSE_STR_BUF_BYTE;
        cur++) {
        
        // store
        if(cur == DATA_CHECKSUM) {
            
        }
        str_array[cur] = token;
        token = strtok(NULL, delim);
    }

    // parse data here
    if(cur == DATA_LEN) {
        
        // parse timestamp
        timestamp = atof(str_array[DATA_TIME]);

        // parse data
        x = atof(str_array[DATA_X]);
        y = atof(str_array[DATA_Y]);
        z = atof(str_array[DATA_Z]);

        // get checksum - string
        checksum = str_array[DATA_CHECKSUM];
        // FIXME: what if DATA_CHECKSUM + 1 out of bound
        str_array[DATA_CHECKSUM + 1] = '\0';    // add a null char after checksum, making strlen enable to work
        parseable = DATA;

    } else if(cur == INFO_LEN) {
        if(!strcmp(str_array[INFO], STR_PASSIVE_DISCONNECT))
            parseable = PASSIVE_DISCONNECT;
        else if(!strcmp(str_array[INFO], STR_ACTIVE_DISCONNECT))
            parseable = ACTIVE_DISCONNECT;
    }

    if(parseable == UNPARSEABLE)
        printf("\n\n## Parse failed ##\n\n");

    else if(parseable == DATA)
        printf("timestamp: %.3f,  x: %.3f, y: %.3f, z: %.3f, checksum : %s\n",timestamp,x,y,z,checksum);
    
    else if(parseable == PASSIVE_DISCONNECT) {
        printf("## Be notified to go disconnection process by server, this process is going to close ##\n");
        looping = 0;
    }
    else {
        // supposed to be empty
        // do nothing
    }

    // reset buf
    memset(rx_buf,0,sizeof(rx_buf));

    return parseable;
}

int main(int argc, char* argv)
{
    /////////////////////////////////
    // user tune - decide server ip
    server_ip = malloc(sizeof(char)*strlen(local_ip));
    strcpy(server_ip, local_ip);
    /////////////////////////////////

    // var region
    extern int errno;

    // build socket 
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);    // IPv4 - TCPIP socket
    if (socket_fd < 0) {
        printf("Fail to create socket\n");
        exit(EXIT_FAILURE);
    }
    
    // server info assign for IPv4, IPv6 use sockaddr_in6
    // IPv6 see here : https://man7.org/linux/man-pages/man7/ipv6.7.html
    struct sockaddr_in server_info = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(server_ip),
        .sin_port = htons(server_port)
    };

    // connection build 
    try_connection(&socket_fd, server_info, max_retry_num, sleep_time);

    // select setting
    fd_set rfd;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    // main loop
    while(looping) {
        if(socket_fd) {
            // recv data
            recv(socket_fd, rx_buf, MAX_BUF_BYTE, 0);

            // parse data
            int parseable = parse(rx_buf, dlm);
            
            // send back checksum or passive disconnection cmd recv signal 
            send_back(parseable, socket_fd);
        }

        // check user input 
        FD_ZERO(&rfd);
        FD_SET(0, &rfd);
        int ret = select(STDIN_FILENO +1, &rfd, NULL, NULL, &tv);
        if (ret < 0) {
            perror("select wrong");
            exit(EXIT_FAILURE);
        }
        // operate cmd
        if (ret) {
            switch(userinput_check())
            {
            case EXIT_PROCESS:
                looping = 0;
                // break; exit_process should send active_disconncet first

            case SEND_DISCONNECT_CMD:
                // active disconnect send here 
                send_back(ACTIVE_DISCONNECT, socket_fd);
                // TODO should wait for recv confirm signal from server then close 
                printf("Wait for confirm signal");
                close(socket_fd);
                printf("Socket fd closed\n");
                socket_fd = 0;
                break;

            case SEND_RECONNECT_CMD:
                // reconnect requirement
                try_connection(&socket_fd, server_info, max_retry_num, sleep_time);
                send_back(ACTIVE_RECONNECT, socket_fd);
            
            default:
                // suppose to get OK
                // do nothing
                break;
            }
        }

        // timer ~ 100 Hz
        usleep(1e4);
    }

    // disconnect
    printf("\n## Leaving main loop, ready to disconnect ##\n");
}
