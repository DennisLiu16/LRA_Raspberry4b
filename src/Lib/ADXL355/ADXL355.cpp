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

ssize_t ADXL355::readSingleByte(uint8_t regaddr, uint8_t* buf)
{
    return readMultiByte(regaddr,buf,1);
}

ssize_t ADXL355::readMultiByte(uint8_t regaddr, uint8_t* buf,ssize_t len)
{
    
    if(len <= RW::RWByteMax)
    {
        uint8_t rcmd = (regaddr << 1) | READ;
        ssize_t ret = write(SPI_fd,&rcmd,1);
        if(ret > 0)
            return read(SPI_fd,buf,len);

        /*something bad happen*/
        print("write readMultiByte rcmd failed\n");
        return 0;
    }
    
    print("readMiltiByte len over RW::RWByteMax = 4096\n");
    return 0;
}

ssize_t ADXL355::ParseAccData(uint8_t* buf, ssize_t len)
{
    //ref : https://github.com/gpvidal/adxl355-arduino/blob/master/adxl355.ino
    //buf input should follow -> x data start, data set (9 bytes) as unit, so you need preprocess first 
    
    assert((len >= LenDataSet) && (len % LenDataSet == 0));
    assert((buf[XDataMarkerPos] & 0b00000011) == AccDataMarker::isX);

    int NumAccUnitParse = 0;

    for(int ind = 0 ; ind < len ; ind += LenDataSet)
    {
        NumAccUnitParse += ParseOneAccDataUnit(buf+ind,LenDataSet);
    }

    return NumAccUnitParse;
}

ssize_t ADXL355::ParseOneAccDataUnit(uint8_t* buf, ssize_t len)
{
    //there is still timestamp problem
    try
    {
        MyAccUnit.intX = (buf[0] << 12) + (buf[1] << 4) + (buf[2] >> 4);
        MyAccUnit.intY = (buf[3] << 12) + (buf[4] << 4) + (buf[5] >> 4);
        MyAccUnit.intZ = (buf[6] << 12) + (buf[7] << 4) + (buf[8] >> 4);

        // data form with two complement
        // so if the int (32bits, data length 20 bits) larger than (1<<19 == 0x80000), we should covert to it's two complement

        if(MyAccUnit.intX >= (1<<19))
            MyAccUnit.intX = ~MyAccUnit.intX + 1;
        
        if(MyAccUnit.intY >= (1<<19))
            MyAccUnit.intY = ~MyAccUnit.intY + 1;

        if(MyAccUnit.intZ >= (1<<19))
            MyAccUnit.intZ = ~MyAccUnit.intZ + 1;

        // add AccUnit to public dqueue
        dq_AccUnitData.push_back(MyAccUnit);

        return true;    // 1 accDataUnit parse successfully
    }
    catch(std::exception &e)
    {
        print("%s\n",e.what());
        return false;
    }
}

ssize_t ADXL355::readFifoDataOnce(uint8_t* buf /*need 3 bytes*/)
{
    // FIFO_DATA at 0x11
    ssize_t ret = readMultiByte(0x11,buf,LenDataAxis);

    if(ret > 0)
    {
        AccDataMarker marker = CheckDataMarker(buf,LenDataAxis);
        switch(marker)
        {   /*write some process if you want to deal with not X axis data problem*/
            default:
                break;
        }
    }

    return ret;
}

ssize_t ADXL355::readFifoDataSetOnce(uint8_t* buf /*need 9 bytes*/)
{
     
}

ADXL355::AccDataMarker ADXL355::CheckDataMarker(uint8_t* buf, ssize_t len)
{
    if(len >= LenDataAxis)
    {
        uint8_t DataMarker = buf[XDataMarkerPos] & /*0b00000011*/
                            ( GETMASK(getLength(x_axis_marker),getStartBit(x_axis_marker)) |
                              GETMASK(getLength(empty_indicator),getStartBit(empty_indicator))
                            );

        switch(DataMarker)
        {
            case isEmpty:
                print("data get from reg_FIFO_DATA is invalid\n");
                break;

            case Err_Len2Short:
                print("datamarker in readFifoDataOnce get Err_Len2Short\n" \
                      "Which means readback from reg_FIFO_DATA < reasonable bytes(3), plz check return value of readback\n"
                );
                break;
            default:
                break;
        }
        
        return static_cast<AccDataMarker>(DataMarker);
    }
    return Err_Len2Short;
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
    ssize_t ret = readSingleByte(SPI_fd,r_single_byte);
    if(ret == 0)
    {
        print("set single bit pair failed -- read \n");
        return;
    }

    *r_single_byte &= (~GETMASK(getLength(ri),getStartBit(ri)));    //  make readmask of origin reg, apply on r_single_byte
    *r_single_byte |= (val & GETMASK(getLength(ri),getStartBit(ri)));   // make writemask of val, apply on val then combine (r_single_byte, val)
    uint8_t wcmd = (regaddr << 1) | WRITE;
    uint8_t w_single_byte[2] = {wcmd,*r_single_byte};

    write(SPI_fd,w_single_byte,2);//maybe two byte
}


