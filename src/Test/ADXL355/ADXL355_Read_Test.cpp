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

    ADXL355 adxl355(ADXL355::Default::spi_channel,ADXL355::Default::spi_speed,ADXL355::Default::spi_mode);

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
            len = adxl355.ParseOneAccDataUnit(tmp,len);
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

    // test setSingleBitPair - failed 
    //adxl355.setSingleBitPair(ADXL355::regIndex::odr_lpf,(1<<4|1));   //should get 1<<4 instead
    

}