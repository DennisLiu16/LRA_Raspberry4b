#include <ADXL355/ADXL355.h>
using namespace LRA_ADXL355;

ADXL355::ADXL355(int channel = Default::spi_channel,int speed = Default::spi_speed,int mode = Default::spi_mode){
    /*init spi*/
    SPI_fd = wiringPiSPISetupMode(channel,speed,mode);
    if(SPI_fd > 0)
        print("open SPI successed\n");
}

ADXL355::~ADXL355()
{
    //pass
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

uint8_t ADXL355::readSingleByte(uint8_t regaddr, uint8_t* buf)
{
    return readMultiByte(regaddr,buf,1);
}

uint8_t ADXL355::readMultiByte(uint8_t regaddr, uint8_t* buf,int len)
{
    uint8_t rcmd = (regaddr << 1) | READ;
    ssize_t ret = write(SPI_fd,&rcmd,1);
    if(ret > 0)
        return read(SPI_fd,buf,len);

    /*something happen*/
    return 0;
}

ADXL355::Acc ADXL355::ParseAccData(uint8_t* buf, int len)
{
    //ref : https://github.com/gpvidal/adxl355-arduino/blob/master/adxl355.ino
    //clean non group data : len correct, find out 
    
    assert((len >= LenDataSet) && (len % LenDataSet == 0));

    //correct FIFO data before ParseAccData -> use buf[2] check
    
    
}

ADXL355::AccData ADXL355::readFifoDataOnce(uint8_t* buf /*need 3 bytes*/)
{
    // need speed up
    // FIFO_DATA at 0x11
    uint8_t ret = readMultiByte(0x11,buf,3);

    if(ret)
        return static_cast<AccData>(buf[2] & 0b00000011); 

    return AccData::Wrong;
}

 uint8_t ADXL355::readFifoDataSetOnce(uint8_t* buf /*need 9 bytes*/)
 {
     
 }

void ADXL355::setSingleReg(uint8_t regaddr,uint8_t val)
{
    uint8_t wcmd = (regaddr << 1) | WRITE;
    uint8_t w_single_byte[2] = {wcmd,val};

    write(SPI_fd,w_single_byte,2);//maybe two byte
}

void ADXL355::setSingleReg(uint8_t regaddr,uint8_t val,uint8_t writemask)
{
    /*writemask for where to write -> 1*/
    uint8_t ret = readSingleByte(SPI_fd,r_single_byte);
    if(ret == 0)
    {
        print("set with mask input failed -- read \n");
        return;
    }
    *r_single_byte &= (~writemask);
    *r_single_byte|=(val & writemask);    //should be ok

    uint8_t wcmd = (regaddr << 1) | WRITE;
    uint8_t w_single_byte[2] = {wcmd,*r_single_byte};

    write(SPI_fd,w_single_byte,2);//maybe two byte
}

void ADXL355::setSingleBitPair(regIndex ri,uint8_t val)
{
    uint8_t regaddr = getAddr(ri);
    uint8_t ret = readSingleByte(SPI_fd,r_single_byte);
    if(ret == 0)
    {
        print("set single bit pair failed -- read \n");
        return;
    }
    //
    *r_single_byte &= (~GETMASK(getLength(ri),getStartBit(ri)));
    *r_single_byte |= (val & GETMASK(getLength(ri),getStartBit(ri)));
    uint8_t wcmd = (regaddr << 1) | WRITE;
    uint8_t w_single_byte[2] = {wcmd,*r_single_byte};

    write(SPI_fd,w_single_byte,2);//maybe two byte
}


