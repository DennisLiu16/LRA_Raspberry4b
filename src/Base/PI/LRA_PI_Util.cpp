#include <inc/PI/LRA_PI_Util.h>


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
PI_I2C::PI_I2C(){

}

PI_I2C::PI_I2C(int slave_id){

    /*You should check slave_id is valid or not here - pass*/
    this->slave_id = slave_id;
}

PI_I2C::~PI_I2C(){
    
};

int* PI_I2C::i2c_init(const char* dev_id = I2C_DEFAULT_DEVICE) noexcept{
    try{
        static int* tmp_port;
        if( (*tmp_port = open(dev_id,O_RDWR)) < 0)
            throw ERR_PI_I2C_CONNECTION_FAILURE;
        if( ioctl(*tmp_port,I2C_SLAVE,this->slave_id) < 0)
            throw ERR_PI_I2C_IOCTL_PERMISSION_DENYIED;
        return tmp_port;
    }
    catch(ErrorType e){
        /*send error to err*/
        err = e;
        cout << Error::getErrorName(e) <<endl;
        return nullptr;
    }
}

uint8_t PI_I2C::i2c_read(int* port,int reg_addr){
    // default length 1byte for uint8_t
    
    try{
        /*set as 1 byte and buffer*/
        uint8_t c[1];
        int length = 1;

        if( port == nullptr ){
            /*port not valid*/
            throw ERR_PI_I2C_NULLPTR;
        }
        if( read(*port,c,length) != length){
            throw ERR_PI_I2C_READ_FAILURE;
        }
        return *c;
    }
    catch(ErrorType e){
        err = e;
        cout << Error::getErrorName(e) <<endl;
        return 0;
    }
}

uint8_t PI_I2C::i2c_write(int* port ,int reg_addr, uint8_t content){

}


