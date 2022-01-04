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

    clock_getres(CLOCK_REALTIME, &tt);
    print("resolution: {}\n", tt.tv_nsec);

    int fd  = wiringPiSPISetupMode(0,(int)1e5,0);
    uint8_t tmp[1] = {(1<<1)|1};
    while(true)
    {
        wiringPiSPIDataRW(0,tmp,1);
        printf("tmp = %d\n",tmp[0]);
    }


    /*
    ADXL355 adxl355(ADXL355::Default::spi_channel,ADXL355::Default::spi_speed,ADXL355::Default::spi_mode);
    uint8_t tmp[1]={12};
    adxl355.setSingleReg(0x2f,1);
    ssize_t readlen = adxl355.readSingleByte(0x11,tmp);
    printf("%ld,%d\n",readlen,tmp[0]);

    */
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
}