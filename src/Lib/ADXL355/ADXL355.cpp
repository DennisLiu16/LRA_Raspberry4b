#include <ADXL355/ADXL355.h>
using namespace LRA_ADXL355;

ADXL355::ADXL355(int channel = Default::spi_channel,int speed = Default::spi_speed,int mode = Default::spi_mode){
    /*init spi*/
    fd = wiringPiSPISetupMode(channel,speed,mode);
    if(fd > 0)
        print("open SPI successed\n");
}

ADXL355::~ADXL355()
{

}

inline uint8_t ADXL355::getAddr(regIndex bIndex)
{
    return static_cast<uint8_t>(addr[bIndex]);
}

inline uint8_t ADXL355::getStartBit(regIndex bIndex)
{
    return static_cast<uint8_t>(startbit[bIndex]);
}

inline uint8_t ADXL355::getLength(regIndex bIndex)
{
    return static_cast<uint8_t>(length[bIndex]);
}

uint8_t ADXL355::readSingleReg(uint8_t regaddr, void* buf)
{
    /*send read cmd*/
    uint8_t rcmd = (regaddr << 1) | READ;
    ssize_t ret = write(fd,&rcmd,1);
    if(ret > 0)
        return read(fd,buf,1);

    /*something happen*/
    return 0;
}

uint8_t ADXL355::readDataOnce()
{
    
}

void ADXL355::setSingleReg(uint8_t regaddr,uint8_t val)
{
    uint8_t wcmd = (regaddr << 1) | WRITE;
    uint8_t w_single_byte[2] = {wcmd,val};

    write(fd,w_single_byte,2);//maybe two byte
}
void ADXL355::setSingleReg(uint8_t regaddr,uint8_t val,uint8_t writemask)
{
    /*writemask for where to write -> 1*/
    uint8_t ret = readSingleReg(fd,r_single_byte);
    if(ret == 0)
    {
        print("set with mask input failed -- read \n");
        return;
    }
    *r_single_byte &= (~writemask);
    *r_single_byte|=(val & writemask);    //should be ok

    uint8_t wcmd = (regaddr << 1) | WRITE;
    uint8_t w_single_byte[2] = {wcmd,*r_single_byte};

    write(fd,w_single_byte,2);//maybe two byte
}
void ADXL355::setSingleBit(uint8_t regindex,uint8_t val)
{
    regIndex ri = static_cast<regIndex>(regindex);
    uint8_t regaddr = getAddr(ri);
    uint8_t ret = readSingleReg(fd,r_single_byte);
    if(ret == 0)
    {
        print("set single bit failed -- read \n");
        return;
    }
    *r_single_byte &= (~GETMASK(getLength(ri),getStartBit(ri)));
    *r_single_byte |= (val & GETMASK(getLength(ri),getStartBit(ri)));
    uint8_t wcmd = (regaddr << 1) | WRITE;
    uint8_t w_single_byte[2] = {wcmd,*r_single_byte};

    write(fd,w_single_byte,2);//maybe two byte
}


