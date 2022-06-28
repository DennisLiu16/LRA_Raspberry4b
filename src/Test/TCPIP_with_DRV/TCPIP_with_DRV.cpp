#include <DRV2605L_TCA/DRV2605L_TCA.h>
#include <ADXL355/ADXL355.h>
#include <TCPIP/TCPIP.h>


// for select
#include <sys/select.h>

// namespace alias
using namespace LRA_ADXL355;
using namespace LRA_DRV2605L_TCA;
using namespace LRA_TCPIP;
using DRV = DRV2605L_TCA;

// DRV var
DRV Xdrv;

// TCP var
Client *client;

// local log var
FILE* fdAcc;
FILE* fdRTP;
FILE* fdTCP;

// flow control var
static bool _running = true;
volatile bool _loop_timer = false;

// irq_func
void irq_test_0()
{
    if(ADXL355::InstanceArray[0] != nullptr)
        ADXL355::InstanceArray[0]->_fifoINTRdyFlag = 1;
}

// timer_trig_func
void timer_flag_up()
{
    _loop_timer = true;
}

// user input check
bool usrin_check()
{
    bool ret = 0;
    string usrin;
    cin >> usrin;

    if(usrin == "q" || usrin == "quit")
        ret = 1;

    client->usrin_check_tcpip(usrin);
    return ret;
}

bool check_main_args(int argc, int version)
{
    switch(version)
    {
        case 1:
            // argv[0] : ./path_to/TCPIP_with_DRV
            // argv[1] : server_ip
            if(argc == 2)
                return true;
            return false;
        break;
    }
    return false;
}

// main
int main(int argc, char* argv[])
{
    /*parse main args*/
    #define V_SERVERIP 1
    bool arg_valid = check_main_args(argc, V_SERVERIP);
    
    // TCPIP var (with main args version)
    char* ip;
    int port = 8787;
    if(arg_valid) {
        ip = (char*)malloc(sizeof(char)*strlen(argv[1]));
        strcpy(ip,argv[1]);
    }else {
        const char local_ip[] = "192.168.0.113";
        ip = (char*)malloc(sizeof(char)*strlen(local_ip));
        strcpy(ip,local_ip);
    }

    flushed_print("Server Ip is {}\n", ip);
    sleep(2);

    // const char local_ip[] = "192.168.0.113";
    // char* ip;
    // ip = (char*)malloc(sizeof(char)*strlen(local_ip));
    // strcpy(ip,local_ip);

    wiringPiSetup();

    //pwm
    
    int buzzer_pin = 1; // Pin 1
    pinMode(buzzer_pin, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(256);
    pwmSetClock(100);   // 54M/255/100 ~ 2100Hz
    pwmWrite(buzzer_pin, 0);
    

    DRV Xdrv(7,0);
    Xdrv.setStandBy(DRV::STANDBY_ready);   
    Xdrv.set6S();
    Xdrv.printAllRegIndex();

    ADXL355::s_Init init_para;
    
    init_para.spi_channel = ADXL355::Default::spi_channel;
    init_para.spi_mode = ADXL355::Default::spi_mode;
    init_para.spi_speed = ADXL355::Default::spi_speed;
    init_para.updateMode = ADXL355::Default::INT_update_mode;
    init_para.updateThread = ADXL355::Default::open_updateThread;
    init_para.acc_range = ADXL355::Value::Range_4g;
    init_para.sampling_rate = ADXL355::Value::SamplingRate_4000;
    init_para.INT_pin = 6;
    init_para.isr_handler = irq_test_0;
    init_para.autoSetOffset = true;
    init_para.SetOffsetDataSize = ADXL355::Default::AVG_data_size;

    ADXL355 adxl355(init_para);

    // open fd
    fdAcc = fopen("/home/ubuntu/LRA_Raspberry4b/data/log/Acc.txt","wb");
    fdRTP = fopen("/home/ubuntu/LRA_Raspberry4b/data/log/RTP.txt","wb");
    fdTCP = fopen("/home/ubuntu/LRA_Raspberry4b/data/log/TCP.txt","wb");

    // select setting
    fd_set rfd;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    const int std_in_fd = 0;

    // wait acc data input to deque
    sleep(1);

    uint8_t cmd = 0;
    timespec start, stop, t_loop, last_t_loop;

    Timer myTimer = Timer();

    myTimer.setInterval(timer_flag_up, 10000u);

    struct sockaddr_in server_info = {};
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = inet_addr(ip);
    server_info.sin_port = htons(port);

    client = new Client(server_info, 5, 5, 1);
    if(!client->try_connection()) {
        exit(EXIT_FAILURE);
    }

    while(_running)
    {
        if(_loop_timer) {  // update Acc, RTP, TCP
            last_t_loop = t_loop;
            clock_gettime(CLOCK_REALTIME, &t_loop);
            clock_gettime(CLOCK_REALTIME, &start);

            size_t accNum = adxl355.dq_fAccUnitData.size();

            ADXL355::fAccUnit tmp;

            // get acc info
            for(size_t index = 0; index < accNum; ++index)
            {
                if(adxl355.dq_fAccUnitData.empty()) // need this protect
                    break;
                tmp = adxl355.dq_pop_front();
                fprintf(fdAcc, "%.3f,%.3f,%.3f,%.3f\n", tmp.time_ms, tmp.fX, tmp.fY, tmp.fZ);
            }

            // recv data from socket
            const char delim = ',';
            Signal sig = client->recv_and_parse(delim);
            client->sig_handler(sig);

            // calculate RTP
            if (sig == Signal::force) {
                fprintf(fdTCP, "%.3f, %.3f, %.3f, %.3f, %d\n", 
                            client->data.timestamp,
                            client->data.x,
                            client->data.y,
                            client->data.z,
                            client->data.checksum
                );
                cmd = (uint8_t)client->data.x;
                Xdrv.setRTP(cmd);

                // update pwm
                pwmWrite(buzzer_pin, cmd);

                flushed_print("time: {:.3f}(ms), new cmd: {}\n", time_diff_ms(&adxl355.adxl355_birth_time,&start) ,cmd);
            }

            timespec t_tmp; 
            clock_gettime(CLOCK_REALTIME, &t_tmp);
            double diff = time_diff_ms(&adxl355.adxl355_birth_time,&t_tmp);
            
            fprintf(fdRTP, "%.3f, %d, %.3f\n", diff, cmd, Xdrv.getOperationFreq());
            fflush(fdAcc);
            fflush(fdRTP);
            fflush(fdTCP);

            //read stdin
            FD_ZERO(&rfd);
            FD_SET(std_in_fd, &rfd);    // stdin is fd 0

            int res = select(std_in_fd + 1, &rfd, NULL, NULL, &tv);
            if(res < 0) {
                perror("select wrong");
            } else if(res) {
                if(usrin_check()) { // leave process
                    timespec now;
                    clock_gettime(CLOCK_REALTIME, &now);
                    flushed_print("\n\nLeave time : {:.3f} (s)\n\n", time_diff_ms(&adxl355.adxl355_birth_time, &now)/1000.0);
                    flushed_print("Leaving Main Thread\n");
                    _running = 0;
                }
            }

            clock_gettime(CLOCK_REALTIME, &stop);
            double time_diff = time_diff_us(&start,&stop);
            double between_two_loop = time_diff_us(&last_t_loop, &t_loop);
            if(time_diff > 100) 
                //flushed_print("loop cost (us): {:.3f} ; loop interval (us): {:.3f}\n", time_diff, between_two_loop);

            // flag reset
            _loop_timer = false;
        }
    }
    //close
    Xdrv.setStandBy(DRV::STANDBY_standby);
    adxl355.StopMeasurement();
    pwmWrite(buzzer_pin, 0);
}