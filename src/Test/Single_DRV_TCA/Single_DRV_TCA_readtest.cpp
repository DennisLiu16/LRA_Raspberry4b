#include<DRV2605L_TCA/DRV2605L_TCA.h>
#include<ADXL355/ADXL355.h>

/*namespace alias*/
using namespace LRA_ADXL355;
using namespace LRA_DRV2605L_TCA;
using DRV = DRV2605L_TCA;

/*irq test*/
void irq_test_0()
{
    if(ADXL355::InstanceArray[0] != nullptr)
        ADXL355::InstanceArray[0]->_fifoINTRdyFlag = 1;
}

DRV2605L_TCA Xdrv;

void signal_handler(int signum)
{
    /*can't use in vs code debug mode?*/
    if(signum == SIGINT)
    {
        print("\nCtrl+C triggered, leaving process\n");
        Xdrv.setGo(false);
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
    ADXL355 adxl355(ADXL355::spi_channel,ADXL355::spi_speed,ADXL355::spi_mode,ADXL355::open_updateThread,ADXL355::INT_update_mode,irq_test_0);
    Xdrv.set6S();
    Xdrv.printAllRegIndex();
}