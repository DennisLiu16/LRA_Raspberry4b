#include <inc/LRA_PI_Util.h>


using namespace LRA_PI_Util;

/*class Data*/

Data::Data(){

}

Data::~Data(){

}

uint8_t Data::store_as_csv(char* file_name){

}

uint8_t Data::store_as_database(char* file_name){

}

/*class PI_I2C*/
PI_I2C::PI_I2C(int slave_id){

    /*You should check slave_id is valid or not here - pass*/
    PI_I2C::slave_id = slave_id;
}

int PI_I2C::i2c_init(char* dev_id) noexcept{
    try{
        int tmp_port;
        if( (tmp_port = open(dev_id,O_RDWR)) )
        {

        }
    }
}

uint16_t PI_I2C::i2c_read(int slave_id,int reg_addr){

}

uint8_t PI_I2C::i2c_write(int slave_id,int reg_addr){

}


