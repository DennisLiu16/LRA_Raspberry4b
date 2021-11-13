#include <DRV2605L/LRA_DRV2605L.h>
using namespace LRA_DRV2605L;

DRV2605L::DRV2605L(){

}

DRV2605L::DRV2605L(int slave_id /*=SLAVE_DEFAULT_ID*/){

    this->slave_id = slave_id;
    /*class constructor*/
    this->data = Data();
}

DRV2605L::~DRV2605L(){
    if(is_init)
    {
        /*close bus*/
        i2c_close(i2c.bus);
    }
}

void DRV2605L::init(){
    try{
        /*port open*/
        int tmp_port = -1;
        if( (tmp_port = i2c_open(I2C_DEFAULT_DEVICE)) == -1){
            /*port null*/
            cout << "Err:i2c port" << endl;
            throw ERR_DRV2605L_OPEN_PORT_FAILURE;
        }

        /*open successed, init i2c info*/
        i2c_init_device(&i2c,tmp_port,slave_id);
        is_init = 1;
    }
    catch(ErrorType e){
        errCode = e;
        cout << Error::getErrorName(e) << endl;
    }
}

ssize_t DRV2605L::read(int reg_addr,void *buf,size_t len){
    /*addr range check*/
    try{
        /*equal to reg_addr>0 && reg_addr <= REG_MAX*/
        if( (reg_addr | REG_MAX - reg_addr) < 0){
            /*out of range*/
            cout << "Out of range. The error reg_addr is " << reg_addr <<endl;
            throw ERR_DRV2605L_REGISTER_ADDRESS_DISMATCH;
        }

        /*send read request*/
        return i2c_read(&i2c,reg_addr,buf,len);;
    }
    catch(ErrorType e){
        errCode = e;
        cout << Error::getErrorName(e) << endl;
        return 0;
    }
}

uint8_t* DRV2605L::read_all()
{
    uint8_t *all = new uint8_t[256];
    i2c_read(&i2c,0x0,&all,256);
    return all;
}

uint8_t DRV2605L::read(int reg_addr)
{
    //bug need to assign a memory address to pointer alway, e.g. new uint8_t()
    uint8_t c = 0;
    read(reg_addr,&c,1);
    return c;
}

uint8_t DRV2605L::write(int reg_addr,I2CDevice content){

}
// use c++ style instead in DRV