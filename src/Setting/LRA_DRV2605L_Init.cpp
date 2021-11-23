#include <iostream>
#include <stdio.h> 
#include <string.h> 
#include <DRV2605L/LRA_DRV2605L.h>

using namespace LRA_DRV2605L; 

int main(){
    wiringPiSetup () ;
    pinMode(25,OUTPUT);
    digitalWrite (25, HIGH);
    DRV2605L x_drv(SLAVE_DEFAULT_ID);
    x_drv.init();

    /*single read test*/
        // uint8_t test;
        // test = x_drv.read(0x00);
        // printf("%x\n",test);
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
        x_drv.hard_reset();
        sleep(2);
        x_drv.run_autoCalibration();
    }
