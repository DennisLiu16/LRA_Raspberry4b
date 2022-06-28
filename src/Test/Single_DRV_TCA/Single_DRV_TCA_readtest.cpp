#include<DRV2605L_TCA/DRV2605L_TCA.h>
#include<ADXL355/ADXL355.h>

#include<sys/select.h>   // using select function

/*namespace alias*/
using namespace LRA_ADXL355;
using namespace LRA_DRV2605L_TCA;
using DRV = DRV2605L_TCA;

// var
DRV2605L_TCA Xdrv;
FILE* fdAcc;
FILE* fdRTP;

static bool _running = true;
volatile bool _loop_timer = false;

/*irq test 0*/
void irq_test_0()
{
    if(ADXL355::InstanceArray[0] != nullptr)
        ADXL355::InstanceArray[0]->_fifoINTRdyFlag = 1;
}

/**/
void timer_flag()
{
    _loop_timer = true;
}

/*quit judge*/
bool quit_check()
{
    string userInput;
    cin >> userInput;
    if(userInput == "q" || userInput == "quit")
        return true;
    flushed_print("Input \" {}\" is invalid \n\n", userInput);
    return false;
}

int main()
{
    wiringPiSetup();
    //pwm
    int buzzer_pin = 1; // Pin 1
    pinMode(buzzer_pin, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
    pwmSetRange(256);
    pwmSetClock(100);   // 54M/255/100 ~ 2100Hz
    pwmWrite(buzzer_pin, 0);

    DRV2605L_TCA Xdrv(7, 0);
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
    
    // write to fd var
    fdAcc = fopen("/home/ubuntu/LRA_Raspberry4b/data/log/Acc.txt","wb");
    fdRTP = fopen("/home/ubuntu/LRA_Raspberry4b/data/log/RTP.txt","wb");

    // select setting
    fd_set rfd;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    const int std_in_fd = 0;

    sleep(1);   // prevent no data in deque

    uint8_t val = 0;
    timespec t1,t2,t_loop,last_t_loop;

    // create a Timer 
    Timer myTimer = Timer();

    myTimer.setInterval(timer_flag,9920u);
    
    while(_running)
    {
        
        if( (_loop_timer)/*timer flag*/)
        {
            last_t_loop = t_loop;
            clock_gettime(CLOCK_REALTIME, &t_loop);
            clock_gettime(CLOCK_REALTIME, &t1);
            //flushed_print("loop time is {:.3f}\n", time_diff_ms(&last_t_loop, &t_loop));
            // get size
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
            // calculate RTP
            
            // set RTP
            val++;
            val = 255;
            Xdrv.setRTP(val);
            pwmWrite(buzzer_pin, val);
            
            timespec t_tmp; 
            clock_gettime(CLOCK_REALTIME, &t_tmp);
            double diff = time_diff_ms(&adxl355.adxl355_birth_time,&t_tmp);
            
            fprintf(fdRTP, "%.3f, %d, %.3f\n", diff, val, Xdrv.getOperationFreq());
            
            // make sure write to file directly
            // https://blog.xuite.net/coke750101/coketech/20842552
            fflush(fdAcc);
            fflush(fdRTP);
            
            /*check stdin*/
            FD_ZERO(&rfd);
            FD_SET(0, &rfd);    // stdin is fd 0
            int ret = select(std_in_fd +1, &rfd, NULL, NULL, &tv);
            if(ret < 0)
            {
                perror("select wrong");
                exit(EXIT_FAILURE);
            }
            if(ret)
            {
                if(quit_check())
                {
                    timespec now;
                    clock_gettime(CLOCK_REALTIME, &now);
                    flushed_print("\n\nLeave time : {:.3f} (s)\n\n", time_diff_ms(&adxl355.adxl355_birth_time, &now)/1000.0);
                    flushed_print("Leaving Main Thread\n");
                    _running = 0;
                }
            }

            clock_gettime(CLOCK_REALTIME, &t2);
            double time_diff = time_diff_us(&t1,&t2);
            if(time_diff > 100)
                flushed_print("cost (us): {:.3f} \n", time_diff);

            // flag reset
            _loop_timer = false;
        }
        
    }
    //close
    Xdrv.setStandBy(DRV::STANDBY_standby);
    adxl355.StopMeasurement();
    pwmWrite(buzzer_pin, 0);
}