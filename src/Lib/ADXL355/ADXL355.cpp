#include <ADXL355/ADXL355.h>
using namespace LRA_ADXL355;

ADXL355::ADXL355(int channel = Default::spi_channel,int speed = Default::spi_speed,int mode = Default::spi_mode){
    /*init spi*/
    SPI_fd = wiringPiSPISetupMode(channel,speed,mode);
    if(SPI_fd > 0)
        print("open SPI successed\n");
    ADXL355::channel = channel;
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

// setting functions 
void ADXL355::resetThisAdxl355()
{
    setSingleReg(getAddr(reset),0x52);  //from data sheet
    print("System reset finished \n");
}


bool ADXL355::getStandByState()
{
    /*change to readSingleBitPair() later*/
    readSingleByte(getAddr(standby));
    return (*readBufPtr | GETMASK(getLength(standby),getStartBit(standby)));
}

void ADXL355::setStandByMode()
{
    setSingleBitPair(standby,1);
}

void ADXL355::setMeasureMode()
{
    setSingleBitPair(standby,0);
}

uint8_t ADXL355::getPartID()
{
    readSingleByte(getAddr(partid));
    return *readBufPtr;
}

ssize_t ADXL355::getAllReg()
{
    //readMultiByte(getAddr(devid_ad),0x2f+1);
}

// basic functions
ssize_t ADXL355::readSingleByte(uint8_t regaddr)
{
    return readMultiByte(regaddr,1);
}

ssize_t ADXL355::readMultiByte(uint8_t regaddr, ssize_t len)
{
    
    if(len <= RW::RWByteMax)
    {
        uint8_t rcmd = (regaddr << 1) | READ;
        *buf = rcmd;

        // clean buf before send rcmd to prevent miswrite
        

        int ret = wiringPiSPIDataRW(channel,buf,len+1);
        if(ret > 0)
            return ret-1;
        /*
        you can't separate write and read owing to SPI SCLK trigger mechanism
            ssize_t ret = write(SPI_fd,&rcmd,1);
            if(ret > 0)
            return read(SPI_fd,buf,len);
        */

        /*something bad happen*/
        print("write readMultiByte rcmd failed\n");
        return 0;
    }
    
    print("readMiltiByte len over RW::RWByteMax = 4095\n");
    return 0;
}

ssize_t ADXL355::ParseAccData(ssize_t len)
{
    
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
        /* << or >> must add () to avoid unexpected behavior */
        /*
            @Fatal Error : 
            ref : https://github.com/gpvidal/adxl355-arduino/blob/master/adxl355.ino
            The method used in ref(arduino one) store data into int first. It's a fatal error. This would misunderstand the MSB meaning.
            For example:
            If you convert (1<<19) to int first, then apply the method listed below. 
            You will get 524288 in decimal. However it should be -524288. 
            Another example :
            If you covert (1<<20) to int firs, then apply the method listed below. It would be -1048576. But it should be 0.
            This behavior happen owing to the MSB of int is not at 19th bit. So parse to int first will lead the number and sign wrong.
            Therefore we should use another method.
            => maybe add GETMASK(20,0)

            @Correct One ref:
            https://github.com/analogdevicesinc/EVAL-ADICUP360/blob/master/projects/ADuCM360_demo_adxl355_pmdz/src/ADXL355.c
            
            @Wrong Method:
            {
                MyAccUnit.intX = (buf[0] << 12) + (buf[1] << 4) + (buf[2] >> 4);
                MyAccUnit.intY = (buf[3] << 12) + (buf[4] << 4) + (buf[5] >> 4);
                MyAccUnit.intZ = (buf[6] << 12) + (buf[7] << 4) + (buf[8] >> 4);    

                // data form with two complement
                // so if the int (32bits, data length 20 bits) larger than (1<<19 == 0x80000 == 524288), we should covert to it's two complement

                // MyAccUnit.intX + ((~MyAccUnit.intX + 1) & GETMASK(20,0)) == 2^20 == 1048576

                if(MyAccUnit.intX >= (1<<19))
                    MyAccUnit.intX = (~MyAccUnit.intX + 1);
                
                if(MyAccUnit.intY >= (1<<19))
                    MyAccUnit.intY = (~MyAccUnit.intY + 1);

                if(MyAccUnit.intZ >= (1<<19))
                    MyAccUnit.intZ = (~MyAccUnit.intZ + 1);
            }
        */
        
        uint32_t uintX = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4) & GETMASK(LenBitAxis,0);    // shift and confirm only 0 to 19th bits meaningful
        uint32_t uintY = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4) & GETMASK(LenBitAxis,0);
        uint32_t uintZ = (buf[6] << 12) | (buf[7] << 4) | (buf[8] >> 4) & GETMASK(LenBitAxis,0);

        // if 19th bit is 1, do two complement 
        if( ( uintX & ( 1<<(LenBitAxis-1) ) ) == 1<<(LenBitAxis-1) )
            MyAccUnit.intX = (uintX | ~GETMASK(LenBitAxis,0));  // should invert
        else
             MyAccUnit.intX = uintX;

        if( ( uintY & ( 1<<(LenBitAxis-1) ) ) == 1<<(LenBitAxis-1) )
            MyAccUnit.intY = (uintY | ~GETMASK(LenBitAxis,0));  // should invert
        else
             MyAccUnit.intY = uintY;
        
        if( ( uintZ & ( 1<<(LenBitAxis-1) ) ) == 1<<(LenBitAxis-1) )
            MyAccUnit.intZ = (uintZ | ~GETMASK(LenBitAxis,0));  // should invert
        else
             MyAccUnit.intZ = uintZ;
        
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

ssize_t ADXL355::readFifoDataOnce(/*need 3 bytes*/)
{
    // FIFO_DATA at 0x11
    // read out will be x1 x2 x3
    ssize_t ret = readMultiByte(0x11, LenDataAxis);

    if(ret > 0)
    {
        AccDataMarker marker = CheckDataMarker(LenDataAxis);
        switch(marker)
        {   /*write some process if you want to deal with not X axis data problem*/
            default:
                break;
        }
    }

    return ret;
}

ssize_t ADXL355::readFifoDataSetOnce(/*need 9 bytes*/)
{
    // FIFO_DATA at 0x11
    // read out will be x1 x2 x3 y1 y2 y3 z1 z2 z3
    ssize_t ret = readMultiByte(0x11, LenDataSet);

    if(ret > 0)
    {
        AccDataMarker marker = CheckDataMarker(LenDataAxis);
        switch(marker)
        {   /*write some process if you want to deal with not X axis data problem*/
            default:
                break;
        }
    }

    return ret;
}

ADXL355::AccDataMarker ADXL355::CheckDataMarker(ssize_t len)
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

    write(SPI_fd,w_single_byte,2);
}

void ADXL355::setSingleReg(uint8_t regaddr,uint8_t val,uint8_t writemask)
{
    /*writemask for where to write -> 1*/
    ssize_t ret = readSingleByte(SPI_fd);
    if(ret < 1)
    {
        print("set with mask input failed -- read \n");
        return;
    }
    *buf = *readBufPtr & (~writemask);       // buf[1] & readmask assign to buf[0]
    *buf|=(val & writemask);    //should be ok

    uint8_t wcmd = (regaddr << 1) | WRITE;
    uint8_t w_single_byte[2] = {wcmd,*buf};

    write(SPI_fd,w_single_byte,2);
}

void ADXL355::setSingleBitPair(regIndex ri,uint8_t val)
{
    uint8_t regaddr = getAddr(ri);
    ssize_t ret = readSingleByte(SPI_fd);
    if(ret == 0)
    {
        print("set single bit pair failed -- read \n");
        return;
    }

    *buf = *readBufPtr & (~GETMASK(getLength(ri),getStartBit(ri)));    //  make readmask of origin reg, apply on buf[1] , that is what you get
    *buf |= (val & GETMASK(getLength(ri),getStartBit(ri)));   // make writemask of val, apply on val then combine (buf[0], val)
    uint8_t wcmd = (regaddr << 1) | WRITE;
    uint8_t w_single_byte[2] = {wcmd,*buf};

    write(SPI_fd,w_single_byte,2);//maybe two byte

    // clear buf
}

uint8_t ADXL355::getSingleBitPair(regIndex ri)
{

}


