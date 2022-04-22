#include <TCPIP/TCPIP.h>
using namespace LRA_TCPIP;

Client::Client()
{
    // pass
}

Client::Client(sockaddr_in sockinfo, uint conn_retry_num, uint conn_duration, bool nonblock_rcv)
{
    // allocate buf size
    rx_buf.reserve(default_buf_len);
    tx_buf.reserve(default_buf_len);

    // copy sockinfo to internal var
    server_info = sockinfo;

    // max retry
    max_retry = conn_retry_num;

    // sleep duration
    duration = conn_duration;

    // nonblock_rcv mode
    nonblock_rcv_flag = nonblock_rcv;
}

Client::Client(const uint buf_len, sockaddr_in sockinfo, uint conn_retry_num, uint conn_duration, bool nonblock_rcv, bool debug)
{
    // allocate buf size
    buflen = buf_len;
    rx_buf.reserve(buf_len);
    tx_buf.reserve(buf_len);

    // copy sockinfo to internal var
    server_info = sockinfo;

    // max retry
    max_retry = conn_retry_num;

    // sleep duration
    duration = conn_duration;

    // nonblock_rcv mode
    nonblock_rcv_flag = nonblock_rcv;

    // debug mode
    debug_mode = debug;
}

bool Client::try_connection()
{
    uint retry_num = 1;

    if(isvalid(socket_fd)) {   // socket_fd check
        print("socket already existed{}, reuse it!\n", socket_fd);
        return true;
    } else {    // make a new socket
        // FIXME:IP version  and flags should can be modified by args
        socket_fd = socket(PF_INET, SOCK_STREAM, 0);
        if(!isvalid(socket_fd)) {
            print("Failed to create socket\n");
            return false;
        } else 
            print("Make a new socket successfully\n\n");
    }

    // FIXME:should add check server_info

    // set socket to nonblock if nonblock_rcv_flag on
    if(nonblock_rcv_flag)
        set_sock_nblock_after_connection();
        
    // try to connect to server
    while(connect(socket_fd, (sockaddr*)&server_info, sizeof(server_info)) < 0) {
        //FIXME:errno EWOULDBLOCK in connect state should be ok
        cout << "Connect to socket fd failed: " << retry_num << endl;
        if (retry_num > max_retry - 1) {
            cout << "Max retry limitation exceeded, connection aborted" << endl;
            return false;
        }

        sleep(duration);
        retry_num++;
    }

    // connection built
    cout << "Connect server " << inet_ntoa(server_info.sin_addr) << ":" << ntohs(server_info.sin_port)
    << " succeeded" << endl;

    return true;
}

Signal Client::recv_and_parse(const char delim)
{
    // recv to rx_buf
    int ret = recv(socket_fd, &rx_buf[0], buflen, 0);
    
    if(ret > 0) {   // parse data
        DType type = DType::unknown;
        // split rx_buf
        vector<string> res = split(rx_buf, delim);
        // determine data length according to res len
        switch((DLen)res.size())
        {
            case DLen::force:
                data.timestamp = stof(res.at((int)data_idx::timestamp));
                data.x = stof(res.at((int)data_idx::x));
                data.y = stof(res.at((int)data_idx::y));
                data.z = stof(res.at((int)data_idx::z));
                data.checksum = stoi(res.at((int)data_idx::checksum));
                type = DType::force;
                break;

            case DLen::info:
                if(res.at((int)data_idx::info) == S_DISCONNECTION) { // server require disconnection
                    type = DType::info_disconnection;
                    break;
                } else if(res.at((int)data_idx::info) == S_RECONNECTION) { // server require reconn
                    type = DType::info_reconnection;
                    break;
                } // else go to default
                
            default:
                break;
        }

        // return related signal
        switch(type)
        {
            case DType::force:
                return Signal::force;
            case DType::info_reconnection:
                return Signal::reconnection;
            case DType::info_disconnection:
                return Signal::disconnection;
        }
        return Signal::unparseable;

    } else if(ret == 0) {   // server close socket
        return Signal::disconnection;

    } else if(ret == -1) {  // recv err
        if(errno == EWOULDBLOCK) {    // no data in tcp buf
            return Signal::empty;
        } else if(errno == EBADF) {  
            return Signal::socket_invalid;
        } else {
            return Signal::recv_err;
        }
    }
}

vector<string> Client::split(const string &s, const char delim)
{
    vector<string> token;
    string str;
    istringstream tokenStream(s.c_str());
    while(getline(tokenStream, str, delim)) {
        token.push_back(str);
    }
    return token;
}

void Client::sig_handler(Signal sig)
{
    switch(sig)
    {
        case Signal::force: // send back checksum
            if(debug_mode) {
                print("timestamp: {0:.3f}(s), x: {1:.3f}(N), y: {2:.3f}(N), z: {3:.3f}(N), checksum: {4}\n",
                        data.timestamp,
                        data.x,
                        data.y,
                        data.z,
                        data.checksum);
            }
            // checksum will be checked by server, than we can get next force data
            tx_buf = to_string(data.checksum);
            send_back();
            break;

        case Signal::empty:
            // do nothing
            break;

        case Signal::unparseable:
            print("**data unparseable**\n");
            tx_buf = S_UNPARSEABLE;
            send_back();
            break;

        case Signal::recv_err:
            print("recv error, socket valid\n");
            // FIXME: How to fix this?
            break;

        case Signal::socket_invalid:
            print("socket invalid\n");
            // FIXME:need to recover socket?
            break;
        
        case Signal::disconnection:
            print("Info: disconnection required from server\n");
            // destroy socket
            close(socket_fd);
            break;

        case Signal::trig_disconnection:
            // TODO:maybe send something or close directly
            // destroy socket
            close(socket_fd);
            break;

        case Signal::reconnection:
            // maybe impossible, it's a server's function
            print("Info: reconnection required from server\n");
            break;

        case Signal::trig_reconnection:
            print("Trig reconnection\n");
            if(try_connection())
                print("Reconnect successfully\n");
            else
                print("Reconnect failed\n");
            break;
        
        default:
            print("invalid socket, plz check connection state\n");
        
    }
}

void Client::usrin_check_tcpip(const string &s)
{
    if(s == "r" || s == "reconnection") { // trig reconnection
        sig_handler(Signal::trig_reconnection);

    } else if(s == "d" || s == "disconnection") { // trig disconnection
        sig_handler(Signal::trig_disconnection);
    }
}

bool Client::isvalid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

bool Client::set_sock_nblock_after_connection()
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags|O_NONBLOCK);
}

void Client::send_back()
{
    size_t slen = 0;
    const char *s = tx_buf.c_str(); // s still can move
    do {    // one should check slen == tx_buf.size()
        slen += send(socket_fd, s + slen, tx_buf.size() - slen, 0);
    } while(slen != tx_buf.size());
}
    
    
   
