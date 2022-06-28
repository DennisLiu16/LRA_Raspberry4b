#include<DRV2605L_TCA/DRV2605L_TCA.h>

#include<sys/select.h>   // using select function

/*namespace alias*/
using namespace LRA_DRV2605L_TCA;
using DRV = DRV2605L_TCA;

// var
DRV2605L_TCA Xdrv;
FILE* fdAcc;
FILE* fdRTP;

static bool _running = true;
volatile bool _loop_timer = false; // This volatile is important in -O3 optimization

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
    // int buzzer_pin = 1; // Pin 1
    // pinMode(buzzer_pin, PWM_OUTPUT);
    // pwmSetMode(PWM_MODE_MS);
    // pwmSetRange(256);
    // pwmSetClock(95);   // 54M/255/100 ~ 2100Hz
    // pwmWrite(buzzer_pin, 0);

    DRV2605L_TCA Xdrv(7, 0);
    Xdrv.reset();
    sleep(2); // wait for calibration
    Xdrv.printAllRegIndex();
    Xdrv.setStandBy(DRV::STANDBY_ready);
    Xdrv.setAutoCalibration();
    sleep(5); // wait for calibration
    Xdrv.getCalibrationResult();
    // Xdrv.printAllRegIndex();
    Xdrv.set6S();
    Xdrv.printAllRegIndex();
    Xdrv.setGo(true);
    // write to fd var
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
    flushed_print("Before loop \n");
    while(_running)
    {
        
        if( (_loop_timer)/*timer flag*/)
        {
            last_t_loop = t_loop;
            clock_gettime(CLOCK_REALTIME, &t_loop);
            clock_gettime(CLOCK_REALTIME, &t1);
            //flushed_print("loop time is {:.3f}\n", time_diff_ms(&last_t_loop, &t_loop));
            
            // set RTP
            val++;
            
            Xdrv.setRTP(val);
            // pwmWrite(buzzer_pin, val); //disable pwn
            
            timespec t_tmp; 
            clock_gettime(CLOCK_REALTIME, &t_tmp);
            
            fprintf(fdRTP, "%d, %.3f\n", val, Xdrv.getOperationFreq());
            
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
    // pwmWrite(buzzer_pin, 0);
}