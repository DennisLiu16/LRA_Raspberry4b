#include <iostream>
#include <stdio.h> 
#include <string.h> 
#include <DRV2605L/LRA_DRV2605L.h>
#include <MPU6050/MPU6050.h>

using namespace LRA_DRV2605L; 
DRV2605L x_drv(25,SLAVE_DEFAULT_ID);
//MPU6050 acc(0x68,1);

void signal_handler(int signum)
{
    /*can't use in vs code debug mode?*/
    if(signum == SIGINT)
    {
        print("\nCtrl+C triggered, leaving process\n");
        if(x_drv.is_init)
            x_drv.stop();
        exit(1);
    }
}

int main(){

    /*register signal*/
    if(signal(SIGINT,signal_handler)==SIG_ERR){
        print("Failed to get signal\n");
    }

    /*set up global*/
    wiringPiSetup () ;

    x_drv.init();

    /*single read test*/
        // uint8_t test;
        // test = x_drv.read(0x00);
        // print("%x\n",test);
    /*print all register test*/
        //x_drv.print_all_register();
    /*read mutiple bytes test*/
        // uint8_t *result = new uint8_t[6] ;
        // x_drv.read(0x0,result,6);
        // for(int i = 0;i < 6 ;i++)
        // {
        //     print("{:x}\n",*(result+i));
        // }
    /*write singletest*/
        // x_drv.write(0x01,0x40);
        // x_drv.print_all_register();
    /*write mutiple byte*/
        // const uint8_t test[2] = {0x01,0x01};
        // const uint8_t* a = test;
        // int ret = x_drv.write(0x03,a,2);
        // x_drv.print_all_register();

    /*try setting use set_LRA_6s()*/
        //x_drv.run_autoCalibration();  //need to change set_LRA_6s first

    /*try RTP mode*/
        while(true)
            x_drv.run_RTPtest();    // change to unsigned in 0x1D
    
    /*try MPU6050*/
        //run in other thread - see MPU6050.cpp _update()
        // while(true)
        // {
        //     usleep(1e6);
        // }
}
