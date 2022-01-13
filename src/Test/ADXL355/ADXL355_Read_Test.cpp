#include <ADXL355/ADXL355.h>
#include <time.h>
using namespace LRA_ADXL355;
int main()
{
    wiringPiSetup ();
    print("Start Reading Test\n");
    
    struct timespec tt;
    struct timespec startt;
    clock_gettime(CLOCK_REALTIME, &startt);

    ADXL355 adxl355(ADXL355::spi_channel,ADXL355::spi_speed,ADXL355::spi_mode,ADXL355::open_updateThread,ADXL355::polling_update_mode);

    clock_getres(CLOCK_REALTIME, &tt);
    print("clock resolution: {} ns\n", tt.tv_nsec);

    adxl355.resetThisAdxl355();

    adxl355.setMeasureMode();

    ADXL355::AccUnit accunit;
    ADXL355::fAccUnit faccunit;
    struct timespec t_required, t_remain,t_main;
    double last_record_time = 0.0;

    print("partid is {}\n",adxl355.getPartID());

    while(1)
    {

        if(!adxl355.dq_AccUnitData.empty())
        {
            accunit = adxl355.dq_pop_front();
            adxl355.ParseAccDataUnit(&accunit,&faccunit);
            clock_gettime(CLOCK_REALTIME, &t_main);
            
            timespec t_now = {.tv_sec = t_main.tv_sec-adxl355.adxl355_birth_time.tv_sec,.tv_nsec = t_main.tv_nsec-adxl355.adxl355_birth_time.tv_nsec};
            double now = (t_now.tv_sec)*1e3 + (t_now.tv_nsec)/1e6;
            
            print("now : {:6.3f}  (ms) |  record time : {:6.3f} (ms) | parse delay : {:6.3f} (ms) | record delay : {:6.3f} (ms)| x = {:6.3f} g | y = {:6.3f} g | z = {:6.3f} g | {}\n",
                  now,
                  faccunit.time_ms,
                  now - faccunit.time_ms,
                  faccunit.time_ms - last_record_time,
                  faccunit.fX,
                  faccunit.fY,
                  faccunit.fZ,
                  adxl355.dq_AccUnitData.size()
                  );   //print out test
            
            // assign last val
            last_record_time = faccunit.time_ms;
        }
    }

    print("Leaving ADXL main thread\n");
}