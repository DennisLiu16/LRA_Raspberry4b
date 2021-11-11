#include <DRV2605L/LRA_DRV2605L.h>
using namespace LRA_DRV2605L;

DRV2605L::DRV2605L(){

}

DRV2605L::DRV2605L(int slave_id /*=SLAVE_DEFAULT_ID*/){

    this->slave_id = slave_id;
    /*class constructor*/
    this->i2c = PI_I2C(this->slave_id);
    this->data = Data();
}

DRV2605L::~DRV2605L(){

}

void DRV2605L::init(){
    /*get port
      ioctl done in i2c_init()
    */
    try{
        if( (i2c.i2c_port = i2c.i2c_init()) == nullptr)
            cout << "Err:i2c port" << endl;
            throw ERR_DRV2605L_NULLPTR;
        
    }
    catch(ErrorType e){
        errCode = e;
        cout << Error::getErrorName(e) << endl;
    }
}

uint8_t DRV2605L::read(int reg_addr){
    /*addr range check*/
    try{
        if( (reg_addr | REG_MAX - reg_addr) < 0){
            /*out of range*/
            cout << "Out of range. The error addr is " << reg_addr <<endl;
            throw ERR_DRV2605L_REGISTER_ADDRESS_DISMATCH;
        }
        /*send read request*/
        return i2c.i2c_read(i2c.i2c_port,reg_addr);
    }
    catch(ErrorType e){
        errCode = e;
        cout << Error::getErrorName(e) << endl;
        return 0;
    }
    

}

uint8_t DRV2605L::write(int reg_addr,uint8_t content){

}