/**
 * @file TCPIP.h
 * @author liusx880630@gmail.com
 * @ref https://kknews.cc/zh-tw/code/pbg2ylj.html
 * @version 0.1
 * @date 2022-04-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _LRA_TCPIP_H_
#define _LRA_TCPIP_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <cstring>
#include <cerrno>

#include <PI/LRA_PI_Util.h> 

extern "C"
{
    #include <fcntl.h>  // check fd valid
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
}

namespace LRA_TCPIP
{
    using namespace std;
    using namespace LRA_PI_Util;

    // info var
    #define S_DISCONNECTION "Disconnection"
    #define S_RECONNECTION "Reconnection"
    #define S_UNPARSEABLE "Unparseable"

    // data index
    enum class data_idx
    {
        // force
        timestamp = 0,
        x,
        y,
        z,
        checksum,
        // info
        info = 0,
    };
    
    // for data len
    enum class DLen
    {
        force = 5,
        info = 1,
    };

    enum class Signal
    {
        unparseable,
        force,
        disconnection,
        reconnection,
        trig_disconnection,     // active disconnect from client end
        trig_reconnection,      // active reconnection from client end
        empty,
        recv_err,
        socket_invalid
    };

    enum class DType
    {
        unknown,
        force,
        info_disconnection,
        info_reconnection,
    };

    class Client
    {

        struct LRA_data {
            float timestamp;
            float x;
            float y;
            float z;
            int checksum;
        };

        public:
        bool debug_mode = 0;
        bool nonblock_rcv_flag = 0;
        struct sockaddr_in server_info;
        struct LRA_data data;
        uint buflen = default_buf_len;
        uint max_retry;
        uint duration;
        
        string rx_buf;
        string tx_buf;

        // constructor
        Client();
        /**
         * @brief Construct a new Client object
         * 
         * @param sockinfo 
         * @param conn_retry_num 
         * @param conn_duration 
         * @param nonblock_rcv only for recv, connect set to be block mode.
         * send will be blocked by send_back()
         */
        Client(sockaddr_in sockinfo, uint conn_retry_num, uint conn_duration, bool nonblock_rcv);

        /**
         * @brief Construct a new Client object
         * 
         * @param buf_len 
         * @param sockinfo 
         * @param conn_retry_num 
         * @param conn_duration 
         * @param nonblock_rcv only for recv, connect set to be block mode.
         * send will be blocked by send_back()
         * @param debug 
         */
        Client(const uint buf_len, sockaddr_in sockinfo, uint conn_retry_num, uint conn_duration, bool nonblock_rcv, bool debug);

        // functions
        /**
         * @brief build connection between server,
         * can set max retry num to auto retry with sec_sleep sleep duration
         * 
         * @return true 
         * @return false 
         */
        bool try_connection();

        /**
         * @brief parse rx_buf and return corresponding signal to decide 
         * what message will be send back to server
         * 
         * @param delim 
         * @return Signal 
         */
        Signal recv_and_parse(const char delim);

        /**
         * @brief split string with specific delim
         * @ref https://recluze.net/2019/04/21/split-string-to-int-vector-in-c/
         * @param s 
         * @param delim 
         * @return vector<string> 
         */
        vector<string> split(const string &s, const char delim);

        /**
         * @brief 
         * 
         * @param sig 
         */
        void sig_handler(Signal sig);

        /**
         * @brief 
         * 
         * @param s 
         */
        void check_tcpip_usrin(const string &s);

        /**
         * @brief Set the socket opt object
         * @note https://stackoverflow.com/questions/38021659/can-a-c-socket-recv-0-bytes-without-the-client-shutting-the-connection
         * @note https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout
         * @return true 
         * @return false 
         */
        bool set_sock_nblock_after_connection();

        /**
         * @brief send back tx_buf through socket_fd
         * 
         */
        void send_back();


        private:
        int socket_fd;
        const uint default_buf_len = 128;
        const uint default_max_retry = 5;

        bool isvalid(int fd);

    };
}
#endif