#include <iostream>
#include <stdio.h> 
#include <string.h> 
#include <DRV2605L/LRA_DRV2605L.h>

using namespace LRA_DRV2605L; 

int main(){
    // cout << "Init start" << endl;
    // int bus;

    // /* Open i2c bus /dev/i2c-0 */
    // if ((bus = i2c_open("/dev/i2c-1")) == -1) {
	//     printf("Error open\n");
    // }
    // I2CDevice myi2c;
    
    // myi2c.bus = bus;
    // myi2c.addr = 0x5a;
    // myi2c.iaddr_bytes = 1;
    // myi2c.page_bytes = 16;


    // unsigned char buffer[256];
    // ssize_t size = sizeof(buffer);
    // memset(buffer, 0, sizeof(buffer));

    // /* From i2c 0x0 address read 256 bytes data to buffer */
    // if ((i2c_read(&myi2c, 0x0, buffer, size)) != size) {

    //     /* Error process */
    //     printf("Error read\n");
    // }


    // int count = 0;
    // for(int i = 0;i<256;i++)
    // {
    //     printf("%.2x  ",(int)buffer[i]);
    //     count++;
    //     if(count > 15){
    //         printf("\n");
    //         count = 0;
    //     }
    // }
    // i2c_close(myi2c.bus);

    DRV2605L x_drv(SLAVE_DEFAULT_ID);
    x_drv.init();

    // uint8_t test;
    // test = x_drv.read(0x00);
    // printf("%x\n",test);
    x_drv.print_all_register();
}
