#include <ADXL355/ADXL355.h>
#include <time.h>
using namespace LRA_ADXL355;

void irq_test_0()
{
    if(ADXL355::InstanceArray[0] != nullptr)
        ADXL355::InstanceArray[0]->_fifoINTRdyFlag = 1;
}

int main()
{
    wiringPiSetup ();
    print("Start Reading Test\n");
    
    struct timespec tt;
    struct timespec startt;
    clock_gettime(CLOCK_REALTIME, &startt);

    // for polling 
    //ADXL355 adxl355(ADXL355::spi_channel,ADXL355::spi_speed,ADXL355::spi_mode,ADXL355::open_updateThread,ADXL355::polling_update_mode,ADXL355::isr_default);
    
    // for interrupt
    ADXL355 adxl355(ADXL355::spi_channel,ADXL355::spi_speed,ADXL355::spi_mode,ADXL355::open_updateThread,ADXL355::INT_update_mode,irq_test_0);

    clock_getres(CLOCK_REALTIME, &tt);
    print("clock resolution: {} ns\n", tt.tv_nsec);

    ADXL355::AccUnit accunit;
    ADXL355::fAccUnit faccunit;
    struct timespec t_required, t_remain,t_main;
    double last_record_time = 0.0;
    double last_now = 0.0;

    ADXL355::fOffset foffset =  adxl355.readOffset();
    print("offset : {:6.5f}, {:6.5f}, {:6.5f}\n",foffset.fX,foffset.fY,foffset.fZ);

    adxl355.readMeasureRange();

    while(1)
    {
        
        if(!adxl355.dq_fAccUnitData.empty())
        {
            faccunit = adxl355.dq_pop_front();
            clock_gettime(CLOCK_REALTIME, &t_main);
            timespec t_now = {.tv_sec = t_main.tv_sec-adxl355.adxl355_birth_time.tv_sec,.tv_nsec = t_main.tv_nsec-adxl355.adxl355_birth_time.tv_nsec};
            double now = (t_now.tv_sec)*1e3 + (t_now.tv_nsec)/1e6;

            double ptime = now - faccunit.time_ms;
            if(ptime > 0.3)
            {
            print("now : {:6.3f}  (ms) |  record time : {:6.3f} (ms) | parse delay : {:6.3f} (ms) | record interval : {:6.3f} (ms)| x = {:6.3f} g | y = {:6.3f} g | z = {:6.3f} g | {}\n",
                  now,
                  faccunit.time_ms,
                  ptime,
                  faccunit.time_ms - last_record_time,
                  faccunit.fX,
                  faccunit.fY,
                  faccunit.fZ,
                  adxl355.dq_fAccUnitData.size()
                  );   //print out test
            }
            
            if(now - last_now > 1000)
            {
                  print("now : {:6.3f}  (s) | parse delay : {:6.3f} (ms) | record interval : {:6.3f} (ms)| x = {:6.3f} g | y = {:6.3f} g | z = {:6.3f} g | {}\n",
                  now/1000,
                  ptime,
                  faccunit.time_ms - last_record_time,
                  faccunit.fX,
                  faccunit.fY,
                  faccunit.fZ,
                  adxl355.dq_fAccUnitData.size()
                  );   //print out test
                last_now += 1000;
            }
                
            // assign last val
            last_record_time = faccunit.time_ms;
            

        }
        
    }
    print("Leaving ADXL main thread\n");
}