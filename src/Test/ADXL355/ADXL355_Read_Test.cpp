#include <ADXL355/ADXL355.h>
#include <time.h>
using namespace LRA_ADXL355;
int main()
{
    wiringPiSetup ();
    //pinMode(10,OUTPUT);
    print("Start Reading Test\n");
    
    struct timespec tt;
    struct timespec startt;
    clock_gettime(CLOCK_REALTIME, &startt);

    ADXL355 adxl355(ADXL355::spi_channel,ADXL355::spi_speed,ADXL355::spi_mode,ADXL355::open_updateThread,ADXL355::polling_update_mode);

    clock_getres(CLOCK_REALTIME, &tt);
    print("clock resolution: {} ns\n", tt.tv_nsec);

    /*
    while(true)
    {
        digitalWrite(10,LOW);
        clock_gettime(CLOCK_REALTIME, &tt);
        ssize_t len = adxl355.readFifoDataSetOnce(tmp);

        if(len > 0)
        {
            len = adxl355.PreParseOneAccDataUnit(tmp,len);
            ADXL355::AccUnit accunit;
            try
            {
                accunit = adxl355.dq_AccUnitData.front();
                adxl355.dq_AccUnitData.pop_front();
                accunit.timestamp = (tt.tv_sec - startt.tv_sec) + (tt.tv_nsec - startt.tv_nsec)/1e9;
            }
            catch(std::runtime_error)
            {
                print("TMD, can't read anything from deque\n");
            }

            print("time = {:.6f}, X = {}, Y = {}, Z = {}\n",accunit.timestamp,accunit.intX,accunit.intY,accunit.intZ);
                
        }
    }
    */

   // test setSingleReg - ok 
   //adxl355.setSingleReg(static_cast<uint8_t>(ADXL355::Addr::OFFSET_X_H),0x51);

    // using wiringPiSPIDataRW write - ok 
    // int fd  = wiringPiSPISetupMode(0,(int)1e7,0);
    // uint8_t tmp[2] = {(0x1e<<0)|0,0x15};
    // wiringPiSPIDataRW(0,tmp,2);

    //try to read offset data - ok
    //tmp need to add 1 byte because of delay
    // int fd  = wiringPiSPISetupMode(0,(int)1e7,0);   //mode 0
    // uint8_t tmp[11] = {((0x1e<<1)|1)};
    // wiringPiSPIDataRW(0,tmp,sizeof(tmp));
    // printf("%d,%d,%d,%d,%d,%d,%d\n",tmp[0],tmp[1],tmp[2],tmp[3],tmp[4],tmp[5],tmp[10]);

    // test setSingleBitPair - ok after change read from wiringPiSPIDataRW();
    // adxl355.setSingleBitPair(ADXL355::regIndex::hpf_corner,((1<<5)|1));   //should get 1<<5 not (1<<5|1) instead (write 80,32) to device in);
    // adxl355.readSingleByte(static_cast<uint8_t>(ADXL355::Addr::FILTER),adxl355.buf);    //should get 32 in buf[1];
    // printf("buf[1] = %d\n",adxl355.buf[1]);

    // test FIFO
        //adxl355.readSingleByte(adxl355.getAddr(adxl355.drdy_off),adxl355.buf);
        //printf("buf[1] = %X\n",adxl355.buf[1]);
        //check standby first -> should be 0 in measurement stage

    //adxl355.setSingleReg(adxl355.getAddr(adxl355.reset),0x52);
    adxl355.resetThisAdxl355();
    //adxl355.setSingleBitPair(adxl355.standby,0);
    adxl355.setMeasureMode();

    ADXL355::AccUnit accunit;
    ADXL355::fAccUnit faccunit;
    struct timespec t_required, t_remain,t_main;
    double last_record_time = 0.0;

    //nano sleep setting
    t_required.tv_nsec = 500L;
    t_required.tv_sec = 0L;

    print("partid is {}\n",adxl355.getPartID());

    while(1)
    {
        timespec test1;
        timespec test2;
        if(!adxl355.dq_AccUnitData.empty())
        {
            accunit = adxl355.dq_pop_front();
            adxl355.ParseAccDataUnit(&accunit,&faccunit);
            clock_gettime(CLOCK_REALTIME, &t_main);
            
            timespec t_now = {.tv_sec = t_main.tv_sec-adxl355.adxl355_birth_time.tv_sec,.tv_nsec = t_main.tv_nsec-adxl355.adxl355_birth_time.tv_nsec};
            double now = (t_now.tv_sec)*1e3 + (t_now.tv_nsec)/1e6;
            
            print("now : {:6.3f}  (ms) |  record time : {:6.3f} (ms) | parse delay : {:6.3f} (ms) | record delay : {:6.3f} (ms)| x = {:6.3f} g | y = {:6.3f} g | z = {:6.3f} g\n",
                  now,
                  faccunit.time_ms,
                  now - faccunit.time_ms,
                  faccunit.time_ms - last_record_time,
                  faccunit.fX,
                  faccunit.fY,
                  faccunit.fZ
                  );   //print out test
            
            // assign last val
            last_record_time = faccunit.time_ms;
        }
    }

    print("Leaving ADXL main thread\n");
    // adxl355.readSingleByte(adxl355.getAddr(adxl355.partid));
    // printf("partid is %d\n",*adxl355.readBufPtr);
    /*
    while(1)
    {
        if(!(nanosleep(&t_required,&t_remain) < 0))
        {
            clock_gettime(CLOCK_REALTIME, &tt);
            ssize_t getLen = adxl355.readFifoDataSetOnce(); // read with data ready
            //ssize_t getLen = adxl355.readMultiByte(adxl355.getAddr(adxl355.xdata3),ADXL355::LenDataSet);
            adxl355.PreParseOneAccDataUnit(adxl355.readBufPtr,getLen);
            accunit = adxl355.dq_AccUnitData.front();
            adxl355.dq_AccUnitData.pop_front();
            
            timespec tmp = {.tv_sec = tt.tv_sec-startt.tv_sec,.tv_nsec = tt.tv_nsec-startt.tv_nsec};
            accunit.timestamp = tmp;
            
            adxl355.ParseAccDataUnit(&accunit,&faccunit);
            //double timestamp = (tt.tv_sec - startt.tv_sec)*1e6 + (tt.tv_nsec - startt.tv_nsec)/1e3;  
            //print("{:6.2f} (us)\n",timestamp);     // for time consumption test
            print("{:6.3f} (ms) x = {:6.3f} g, y = {:6.3f} g, z = {:6.3f} g\n",faccunit.time_ms,faccunit.fX,faccunit.fY,faccunit.fZ);   //print out test
        }
    }
    */
   
    


    

}