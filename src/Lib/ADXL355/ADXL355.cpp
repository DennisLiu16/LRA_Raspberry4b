#include <ADXL355/ADXL355.h>
using namespace LRA_ADXL355;

/* init static variable */
ADXL355* ADXL355::InstanceArray[ADXL355::MAX_INSTANCE_NUM] = {nullptr};

ADXL355::ADXL355(int channel,
                 int speed,
                 int mode,
                 bool updateThread,
                 bool updateMode,
                 void (*isr_handler)(void)
                 )
    {

    /* put this pointer into InstanceArray*/
    int i = 0;
    while( i < MAX_INSTANCE_NUM)
    {
        if(InstanceArray[i] == nullptr)
        {
            _thisInstanceIndex = i;
            break;
        }
    }
    if(_thisInstanceIndex < 0)
    {
        print("Instance array need to be enlarged\n");
        return;
    }
        
    InstanceArray[_thisInstanceIndex] = this;

    /* init spi */
    SPI_fd = wiringPiSPISetupMode(channel,speed,mode);
    if(SPI_fd > 0)
        print("Instance {} open SPI successed\n",_thisInstanceIndex + 1);

    _channel = channel;
    _updateMode = updateMode;
    _updateThread = updateThread;

    /*Init setting*/
    clock_gettime(CLOCK_REALTIME, &adxl355_birth_time);

    resetThisAdxl355();
    print("partid is {}\n",getPartID());

    struct timespec tt;
    clock_getres(CLOCK_REALTIME, &tt);
    print("clock resolution: {} ns\n", tt.tv_nsec);

    setMeasureMode();

    if(_updateMode == INT_update_mode)
    {
        /*You should set your interrupt pin here or before thread initial and make sure wiringPiSetup () in your main thread first;*/
        int INT_PIN1 = Default::INT1;

        pinMode(INT_PIN1, INPUT);
        pullUpDnControl(INT_PIN1,PUD_DOWN);
    
        wiringPiISR(INT_PIN1,INT_EDGE_RISING,isr_handler);

        // set adxl355 register to enable interrupt
        // use DRDY pin - active high

    }

    // get offset and tune 
    setOffset();

    //Thread para
    if(_updateThread)
    {
        _exitThread = 0;
        std::thread(&ADXL355::_updateInBackground,this).detach();
    }
        
}

ADXL355::~ADXL355()
{
    //pass
    _exitThread = 1;
    InstanceArray[_thisInstanceIndex] = nullptr;
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
    StopMeasurement();
    setSingleReg(getAddr(reset),0x52);  //from data sheet
    StartMeasurement();
    print("System reset finished \n");
}


bool ADXL355::getStandByState()
{
    /*change to readSingleBitPair() later*/
    StopMeasurement();
    readSingleByte(getAddr(standby));
    uint8_t u8read = *readBufPtr;
    StartMeasurement();
    return (u8read | GETMASK(getLength(standby),getStartBit(standby)));
}

void ADXL355::setStandByMode()
{
    StopMeasurement();
    setSingleBitPair(regIndex::standby,1);
}

void ADXL355::setMeasureMode()
{
    setSingleBitPair(regIndex::standby,0);
    StartMeasurement(); //start immediately
}

uint8_t ADXL355::readMeasureRange()
{
    StopMeasurement();
    uint8_t val = getSingleBitPair(regIndex::range);
    switch(val)
    {
        case Value::Range_2g:
            print("current measure range is ± 2g\n");
            break;
        case Value::Range_4g:
            print("current measure range is ± 4g\n");
            break;
        case Value::Range_8g:
            print("current measure range is ± 8g\n");
            break;
        default:
            print("get measure range failed, plz check \n");
    }
    StartMeasurement();
    return val;
}

void ADXL355::setMeasureRange(uint8_t val)
{
    StopMeasurement();

    /*val should be 0b10, 0b01 or 0b11*/
    switch(val)
    {
        case Value::Range_2g:
            print("set measure range to ± 2g\n");
            break;
        case Value::Range_4g:
            print("set measure range to ± 4g\n");
            break;
        case Value::Range_8g:
            print("set measure range to ± 8g\n");
            break;
        default:
            print("range out of range, set to ± 2g\n");
            val = Value::Range_2g;
    }

    setSingleBitPair(regIndex::range, val);
    StartMeasurement();
}

uint8_t ADXL355::getPartID()
{
    StopMeasurement();
    readSingleByte(getAddr(partid));
    uint8_t u8read = *readBufPtr;
    StartMeasurement();
    return u8read;
}

ssize_t ADXL355::getAllReg()
{
    //readMultiByte(getAddr(devid_ad),0x2f+1);
}

ADXL355::fOffset ADXL355::readOffset()
{
    StopMeasurement();
    
    readMultiByte(getAddr(offset_x_h),LenOffsetSet);

    // no read buf protection
    uint16_t uintX = (*readBufPtr << 8) | *(readBufPtr+1);
    uint16_t uintY = (*(readBufPtr+2) << 8) | *(readBufPtr+3);
    uint16_t uintZ = (*(readBufPtr+4) << 8) | *(readBufPtr+5);

    int16_t intX = uintX, intY = uintY, intZ = uintZ;

    // owing to it's a 16 bits num, so copy uint to int directly , int will deal with positive and negative issue automatically)

    //parse to double
    fOffset foffset;
    foffset.fX =  (double)intX * (1.0 / offset_adc_num) * AccMeasureRange;
    foffset.fY =  (double)intY * (1.0 / offset_adc_num) * AccMeasureRange;
    foffset.fZ =  (double)intZ * (1.0 / offset_adc_num) * AccMeasureRange;

    StartMeasurement();

    return foffset;
}

void ADXL355::setOffset()
{
    StopMeasurement();
    
    // read for about 1 sec 
    timespec front;
    timespec end;
    timespec diff;
    clock_gettime(CLOCK_REALTIME,&front);
    clock_gettime(CLOCK_REALTIME,&end);
    // get avg 

    while(dq_fAccUnitData.size() < AVG_data_size)
    {
        if( (_updateMode == polling_update_mode) || _fifoINTRdyFlag)
        {
            uint8_t tmp_buf[LenDataSet+1];  //include return 0 at first byte -> make data restore to a different buf in this thread 

            //ssize_t _len = readFifoDataSetOnce();
            //PreParseOneAccDataUnit(readBufPtr,_len);    //origin

            ssize_t _len = readAxesDataOnce(tmp_buf);
            //ssize_t _len = readFifoDataSetOnce(tmp_buf);
            PreParseOneAccDataUnit(tmp_buf+1,_len,TypeAxes);    //readBufPtr here safe?
            //PreParseOneAccDataUnit(tmp_buf+1,_len,TypeFifo); // for type fifo

            _fifoINTRdyFlag = 0;
        }
    }

    clock_gettime(CLOCK_REALTIME,&end);

    print("Collect {} data take {:6.3f} (ms)\n",AVG_data_size,time_diff_ms(&front,&end));

    clock_gettime(CLOCK_REALTIME,&front);
    clock_gettime(CLOCK_REALTIME,&end);


    // avg
    fAccUnit faccunit = 
    {
        .fX = 0.0,
        .fY = 0.0,
        .fZ = 0.0
    };
    
    while(!dq_fAccUnitData.empty())
        faccunit += dq_pop_front();

    faccunit/=AVG_data_size;    // 
    
    clock_gettime(CLOCK_REALTIME,&end);

    print("Calculate average take {:6.3f} (us)\n",time_diff_us(&front,&end));
    print("fX : {:6.3f}, fY : {:6.3f}, fZ : {:6.3f} \n",faccunit.fX,faccunit.fY,faccunit.fZ);

    fOffset foffset;
    foffset.fX = faccunit.fX;
    foffset.fY = faccunit.fY;
    foffset.fZ = faccunit.fZ;   

    // set offset
    setOffset(foffset);

    StartMeasurement();
}

void ADXL355::setOffset(ADXL355::fOffset foffset)
{
    StopMeasurement();

    int16_t intX = round(foffset.fX / AccMeasureRange * offset_adc_num);
    int16_t intY = round(foffset.fY / AccMeasureRange * offset_adc_num);
    int16_t intZ = round(foffset.fZ / AccMeasureRange * offset_adc_num);

    //https://www.pupuliao.info/2014/06/cc-%E5%88%A9%E7%94%A8%E4%BD%8D%E5%85%83%E9%81%8B%E7%AE%97%E5%8A%A0%E9%80%9F%E9%81%8B%E7%AE%97%E6%95%88%E7%8E%87/

    if( ABS(intX) > (offset_adc_num >> 1) ||    // offset_adc_num / 2
        ABS(intY) > (offset_adc_num >> 1) ||
        ABS(intZ) > (offset_adc_num >> 1) )
    {
        // out of range
        print("offset set out of range \n");
        return;
    }

    uint8_t tmp_buf[LenOffsetSet] = {
        static_cast<uint8_t>(intX >> 8),
        static_cast<uint8_t>(intX),
        static_cast<uint8_t>(intY >> 8),
        static_cast<uint8_t>(intY),
        static_cast<uint8_t>(intZ >> 8),
        static_cast<uint8_t>(intZ),
    };

    setMultiByte(getAddr(offset_x_h),LenOffsetSet,tmp_buf);

    StartMeasurement();
}

void ADXL355::StartMeasurement()
{
    if(_updateThread && !_doMeasurement)
        _doMeasurement = true;
}

void ADXL355::StopMeasurement()
{
    if(_updateThread && _doMeasurement)
    {
        _doMeasurement = false;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));  // wait for measure thread done one loop
    }
}

void ADXL355::isr_default()
{
    // do nothing for default is polling mode
    // if you want to use interrupt mode, build one irq_handler in the main thread 
    // see example 
}

// thread functions
void ADXL355::dq_push_back(const fAccUnit _faccunit)
{
    std::lock_guard<std::mutex> dq_push_back_lock(deque_mutex);
    dq_fAccUnitData.push_back(_faccunit);
}

ADXL355::fAccUnit ADXL355::dq_pop_front()
{
    std::lock_guard<std::mutex> dq_pop_front_lock(deque_mutex);
    fAccUnit _faccunit = dq_fAccUnitData.front();
    dq_fAccUnitData.pop_front();
    return _faccunit;
}

void ADXL355::_updateInBackground()
{
    print("Start ADXL355 update in background\n");

    /*init parameters*/
    AccUnit _accunit;
    fAccUnit _faccunit;

    /*update information*/
    while(!_exitThread)
    {
        if(_doMeasurement)
        {
            // maybe read out old data here in first run 
            if( (_updateMode == polling_update_mode) || _fifoINTRdyFlag)
            {
                uint8_t tmp_buf[LenDataSet+1];  //include return 0 at first byte -> make data restore to a different buf in this thread 

                ssize_t _len = readAxesDataOnce(tmp_buf);
                //ssize_t _len = readFifoDataSetOnce(tmp_buf);
                PreParseOneAccDataUnit(tmp_buf+1,_len,TypeAxes);    
                //PreParseOneAccDataUnit(tmp_buf+1,_len,TypeFifo); // for type fifo

                _fifoINTRdyFlag = 0;
            }
        }
    }
    print("Leaving update thread\n");
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
        
        int ret = wiringPiSPIDataRW(_channel,buf,len+1);
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

ssize_t ADXL355::readMultiByte(uint8_t regaddr, ssize_t len, uint8_t *tmp_buf)
{
    
    if(len <= RW::RWByteMax)
    {
        uint8_t rcmd = (regaddr << 1) | READ;
        *tmp_buf = rcmd;

        // clean buf before send rcmd to prevent miswrite

        int ret = wiringPiSPIDataRW(_channel,tmp_buf,len+1);
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

ssize_t ADXL355::setMultiByte(uint8_t regaddr, ssize_t len,uint8_t* tmp_buf)
{
    if(len <= RW::RWByteMax)
    {
        uint8_t wcmd = (regaddr << 1) | WRITE;
        uint8_t cmd_buf[len+1];
        *cmd_buf = wcmd;
        memcpy(cmd_buf+1,tmp_buf,len);

        int ret = wiringPiSPIDataRW(_channel,cmd_buf,len+1);
        if(ret > 0)
            return ret-1;

        print("write setMultiByte wcmd failed\n");
        return 0;
    }
    print("setMiltiByte len over RW::RWByteMax = 4095\n");
    return 0;
}

ssize_t ADXL355::ParseAccDataUnit(AccUnit* _accUnit, fAccUnit* _faccUnit)
{
    // (2^20/2) == 524288 == acc_adc_num
    _faccUnit->time_ms = _accUnit->timestamp.tv_nsec * 1e-6 + _accUnit->timestamp.tv_sec * 1e3;
    _faccUnit->fX = ((double)_accUnit->intX) * (1.0/ acc_adc_num) * AccMeasureRange; 
    _faccUnit->fY = ((double)_accUnit->intY) * (1.0/ acc_adc_num) * AccMeasureRange; 
    _faccUnit->fZ = ((double)_accUnit->intZ) * (1.0/ acc_adc_num) * AccMeasureRange; 

    return true;
}

ssize_t ADXL355::PreParseAccData(ssize_t len)
{
    
    //buf input should follow -> x data start, data set (9 bytes) as unit, so you need preprocess first 
    
    assert((len >= LenDataSet) && (len % LenDataSet == 0));
    assert((buf[XDataMarkerPos+1] & 0b00000011) == AccDataMarker::isX );

    int NumAccUnitParse = 0;

    for(int ind = 0 ; ind < len ; ind += LenDataSet)
    {
        NumAccUnitParse += PreParseOneAccDataUnit(buf+ind,LenDataSet,TypeFifo);
    }

    return NumAccUnitParse;
}

ssize_t ADXL355::PreParseOneAccDataUnit(const uint8_t* buf, ssize_t len, int type)
{
    //tiem stamp should add 


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
                _MyAccUnit.intX = (buf[0] << 12) + (buf[1] << 4) + (buf[2] >> 4);
                _MyAccUnit.intY = (buf[3] << 12) + (buf[4] << 4) + (buf[5] >> 4);
                _MyAccUnit.intZ = (buf[6] << 12) + (buf[7] << 4) + (buf[8] >> 4);    

                // data form with two complement
                // so if the int (32bits, data length 20 bits) larger than (1<<19 == 0x80000 == 524288), we should covert to it's two complement

                // _MyAccUnit.intX + ((~_MyAccUnit.intX + 1) & GETMASK(20,0)) == 2^20 == 1048576

                if(_MyAccUnit.intX >= (1<<19))
                    _MyAccUnit.intX = (~_MyAccUnit.intX + 1);
                
                if(_MyAccUnit.intY >= (1<<19))
                    _MyAccUnit.intY = (~_MyAccUnit.intY + 1);

                if(_MyAccUnit.intZ >= (1<<19))
                    _MyAccUnit.intZ = (~_MyAccUnit.intZ + 1);
            }
        */
        // confirm data valid -- if you use polling not INT

        
        if(type == TypeFifo)
        {
            uint8_t datamarker = (buf[2] | buf[5] | buf[8]) & GETMASK(DataMarkerLen,0);
            if( datamarker != ADXL355::isX)
                return false;
        }
        
        uint32_t uintX = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4) & GETMASK(LenBitAxis,0);    // shift and confirm only 0 to 19th bits meaningful
        uint32_t uintY = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4) & GETMASK(LenBitAxis,0);
        uint32_t uintZ = (buf[6] << 12) | (buf[7] << 4) | (buf[8] >> 4) & GETMASK(LenBitAxis,0);

        // if 19th bit is 1, do two complement 
        if( ( uintX & ( 1<<(LenBitAxis-1) ) ) == 1<<(LenBitAxis-1) )
            _MyAccUnit.intX = (uintX | ~GETMASK(LenBitAxis,0));  // should invert
        else
            _MyAccUnit.intX = uintX;

        if( ( uintY & ( 1<<(LenBitAxis-1) ) ) == 1<<(LenBitAxis-1) )
            _MyAccUnit.intY = (uintY | ~GETMASK(LenBitAxis,0));  // should invert
        else
            _MyAccUnit.intY = uintY;
        
        if( ( uintZ & ( 1<<(LenBitAxis-1) ) ) == 1<<(LenBitAxis-1) )
            _MyAccUnit.intZ = (uintZ | ~GETMASK(LenBitAxis,0));  // should invert
        else
            _MyAccUnit.intZ = uintZ;

        ParseAccDataUnit(&_MyAccUnit,&_MyfAccUnit);
        
        dq_push_back(_MyfAccUnit);

        return true;    // 1 faccDataUnit parse successfully
    }
    catch(std::exception &e)
    {
        print("%s\n",e.what());
        return false;
    }
}

ssize_t ADXL355::readAxesDataOnce(uint8_t* tmp_buf/*9+1 bytes*/)
{
    //update timestamp
    timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    _t.tv_sec -= adxl355_birth_time.tv_sec;
    _t.tv_nsec-= adxl355_birth_time.tv_nsec;

    _MyAccUnit.timestamp = _t;

    // X_3 at 0x08
    ssize_t ret = readMultiByte(0x08, LenDataSet, tmp_buf);

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

ssize_t ADXL355::readFifoDataOnce(/*need 3 bytes*/)
{
    // FIFO_DATA at 0x11
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
    //update timestamp
    timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    _t.tv_sec -= adxl355_birth_time.tv_sec;
    _t.tv_nsec-= adxl355_birth_time.tv_nsec;

    _MyAccUnit.timestamp = _t;

    // FIFO_DATA at 0x11
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

ssize_t ADXL355::readFifoDataSetOnce(uint8_t* tmp_buf/*need 9+1 bytes*/)
{
    //update timestamp
    timespec _t;
    clock_gettime(CLOCK_REALTIME, &_t);
    _t.tv_sec -= adxl355_birth_time.tv_sec;
    _t.tv_nsec-= adxl355_birth_time.tv_nsec;

    _MyAccUnit.timestamp = _t;

    // FIFO_DATA at 0x11
    ssize_t ret = readMultiByte(0x11, LenDataSet, tmp_buf);

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
        uint8_t DataMarker = buf[XDataMarkerPos+1] & /*0b00000011*/
                            ( GETMASK(getLength(x_axis_marker),getStartBit(x_axis_marker)) |
                              GETMASK(getLength(empty_indicator),getStartBit(empty_indicator))
                            );

        switch(DataMarker)
        {
            case isEmpty:
                //print("data get from reg_FIFO_DATA is invalid\n");
                //-->not print out
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
    ssize_t ret = readSingleByte(regaddr);
    if(ret == 0)
    {
        print("set single bit pair failed -- read stage \n");
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
    uint8_t regaddr = getAddr(ri);
    ssize_t ret = readSingleByte(regaddr);
    if(ret == 0)
    {
        print("get single bit pair failed -- read stage \n");
        return 0;
    }

    *buf = *readBufPtr & GETMASK(getLength(ri),getStartBit(ri));

    /* get value of bit pair */
    return (*buf >> getStartBit(ri));
}


