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
            throw ERR_DRV2605L_OPEN_PORT_FAILURE;
        }

        /*open successed, init i2c info*/
        i2c_init_device(&i2c,tmp_port,slave_id);
        is_init = 1;
    }
    catch(ErrorType e){
        errCode = e;
        format("{}\n",Error::getErrorName(e));
    }
}

ssize_t DRV2605L::read(uint32_t reg_addr,void *buf,size_t len){
    /*addr range check*/
    try{
        /*equal to reg_addr>0 && reg_addr <= REG_MAX*/
        if( (reg_addr | REG_MAX - reg_addr) < 0){
            /*out of range*/
            format("Out of range. The error reg_addr is {:#04x}",reg_addr);
            throw ERR_DRV2605L_REGISTER_ADDRESS_DISMATCH;
        }

        /*send read request*/
        return i2c_read(&i2c,reg_addr,buf,len);;
    }
    catch(ErrorType e){
        errCode = e;
        format("{}\n",Error::getErrorName(e));
        return -1;
    }
}

void DRV2605L::print_all_register()
{
    /*print only useful registers*/
    uint8_t all[REG_NUM];
    i2c_read(&i2c,0x0,&all,REG_NUM);

    auto isSame = [](uint8_t a,uint8_t b){if(a!=b) return 'x';return ' ';};

    /*show in terminal*/
    int col_width = 20;
    /*print column's names , 15 char per column, fmt key - seperated*/
    print(emphasis::bold | fg(color::yellow),
        "{0:>{5}}{1:>{5}}{2:>{5}}{3:>{5}}{4:>{5}}{6:>{5}}\n",
        "Register_Addr",
        "Hex_Value",
        "Default_Hex",
        "Binary_Value",
        "Default_Binary",
        col_width,
        "isSame");

    for(uint8_t i = 0; i < REG_NUM; i++)
    {
        print( "{0:>{5}}{1:>{5}}{2:>{5}}{3:>{5}}{4:>{5}}{6}\n",
        format("{:#04x}",i),
        format("{:02x}",all[i]),
        format("{:02x}",Default_Value[i]),
        format("{:08b}",all[i]),
        format("{:08b}",Default_Value[i]),
        col_width,
        format(emphasis::bold | fg(color::yellow),"{0:>{1}}",isSame(all[i],Default_Value[i]), col_width)
        );
    }
    return;
}

uint8_t DRV2605L::read(uint32_t reg_addr)
{
    //bug need to assign a memory address to pointer alway, e.g. new uint8_t()
    uint8_t c = 0;
    read(reg_addr,&c,1);
    return c;
}

ssize_t DRV2605L::write(uint32_t reg_addr,const void* content, size_t len){

}

ssize_t DRV2605L::write(uint32_t reg_addr,uint8_t content)
{
    try{
        /*equal to reg_addr>=1 && reg_addr <= REG_MAX*/
        if( ((reg_addr-1) | (REG_MAX - reg_addr)) < 0){
            /*out of range*/
            format("Out of range. The error reg_addr is {:#04x}",reg_addr);
            throw ERR_DRV2605L_REGISTER_ADDRESS_DISMATCH;
        }

        /*send write request*/
        const uint8_t* c = &content;
        return i2c_write(&i2c,reg_addr,c,1);
    }
    catch(ErrorType e){
        errCode = e;
        format("{}\n",Error::getErrorName(e));
        return -1;
    }
    
}
