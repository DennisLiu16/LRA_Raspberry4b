#include<DRV2605L_TCA/DRV2605L_TCA.h>
#include<ADXL355/ADXL355.h>

#include<unistd.h>  // for write

/*namespace alias*/
using namespace LRA_ADXL355;
using namespace LRA_DRV2605L_TCA;
using DRV = DRV2605L_TCA;

// var
DRV2605L_TCA Xdrv;
FILE* fdAcc;
FILE* fdRTP;

/*irq test*/
void irq_test_0()
{
    if(ADXL355::InstanceArray[0] != nullptr)
        ADXL355::InstanceArray[0]->_fifoINTRdyFlag = 1;
}

/*Crtl+C related*/
void signal_handler(int signum)
{
    /*can't use in vs code debug mode?*/
    if(signum == SIGINT)
    {
        print("\nCtrl+C triggered, leaving process\n");
        Xdrv.setStandBy(DRV::STANDBY_standby);
        string s = Xdrv.getBitPair(DRV::STANDBY) ? "Standby" : "Ready";
        print("{}",s);
        sleep(1);
        fclose(fdAcc);
        fclose(fdRTP);
        exit(1);
    }
}

int main()
{
    /*register signal*/
    if(signal(SIGINT,signal_handler)==SIG_ERR){
        print("Failed to get signal\n");
    }

    wiringPiSetup();
    DRV2605L_TCA Xdrv(25, 0);
    Xdrv.setStandBy(DRV::STANDBY_ready);   
    Xdrv.set6S();
    Xdrv.printAllRegIndex();
    ADXL355 adxl355(ADXL355::spi_channel,ADXL355::spi_speed,ADXL355::spi_mode,ADXL355::open_updateThread,ADXL355::INT_update_mode,irq_test_0);

    // write to fd var
    fdAcc = fopen("Acc.txt","wb");
    fdRTP = fopen("RTP.txt","wb");

    sleep(1);

    uint8_t val = 0;

    while(true)
    {
        if( (adxl355.dq_fAccUnitData.size() > 40)/*timer flag*/)
        {
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

            Xdrv.setRTP(val);

            timespec t_tmp; 
            clock_gettime(CLOCK_REALTIME, &t_tmp);
            double diff = time_diff_ms(&adxl355.adxl355_birth_time,&t_tmp);

            fprintf(fdRTP, "%.3f, %d, %.3f\n", diff, val, Xdrv.getOperationFreq());
            //print("now {:^.3f} (ms) | acc_now : {:^.3f} (ms)\n", diff, tmp.time_ms);

            // make sure write to file directly
            // https://blog.xuite.net/coke750101/coketech/20842552
            fflush(fdAcc);
            fflush(fdRTP);
            // write to file or db
        }
    }
    Xdrv.setStandBy(DRV::STANDBY_standby);
}