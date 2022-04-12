// for data array size
#define MAX_BUF_BYTE 100
#define PARSE_STR_BUF_BYTE 10

// for data len
#define DATA_LEN 5  // Time, x, y, z, checksum
#define INFO_LEN 1

// for data frame structure
/*Single Byte*/
#define INFO 0
/*DATA*/
#define DATA_TIME 0
#define DATA_X (DATA_TIME+1)
#define DATA_Y (DATA_X+1)
#define DATA_Z (DATA_Y+1)
#define DATA_CHECKSUM (DATA_Z+1)

//for send_back
#define UNPARSEABLE 0
#define DATA 1
#define PASSIVE_DISCONNECT 2
#define ACTIVE_DISCONNECT 3
#define ACTIVE_RECONNECT 4

// for send_back str
#define STR_PASSIVE_DISCONNECT "PASSIVE_DISCONNECT"
#define STR_ACTIVE_DISCONNECT "ACTIVE_DISCONNECT"
#define STR_ACTIVE_RECONNECT "ACTIVE_RECONNECT"

// for user input 
#define OK 0
#define EXIT_PROCESS 1
#define SEND_DISCONNECT_CMD 2
#define SEND_RECONNECT_CMD 3

// for TCPIP
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

// for errno
#include <errno.h>
// for timeval
#include <sys/time.h>
// for select
#include <signal.h>
#include <sys/select.h>
// for pthread
#include <pthread.h>

// struct define
struct client_info{
    int sockfd;
    struct sockaddr_in clientAddr;
};