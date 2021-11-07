// sys include - c++
#include <iostream>
#include <string>
// sys include - c
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// self include 
#include <LRA_DRV2605L.h>
#include <LRA_ErrorCode.h>

//functions
int i2c_interface_init(char* i2c_target) noexcept;
// var
int i2c_port;
 
void main(){
    i2c_port = i2c_interface_init(I2C_DEFAULT_TARGET);
}

int i2c_interface_init(char* i2c_target) noexcept{

    int tmp_port;

    
}