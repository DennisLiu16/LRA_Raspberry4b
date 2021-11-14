#include <iostream>
#include <stdio.h> 
#include <string.h> 
#include <DRV2605L/LRA_DRV2605L.h>

using namespace LRA_DRV2605L; 

int main(){
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
        x_drv.write(0x03,0x06);
        x_drv.print_all_register();
    }
