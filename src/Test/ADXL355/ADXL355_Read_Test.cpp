#include <ADXL355/ADXL355.h>
using namespace LRA_ADXL355;

void irq_test_0()
{
    if(ADXL355::InstanceArray[0] != nullptr)
        ADXL355::InstanceArray[0]->_fifoINTRdyFlag = 1;
}

int main()
{
    wiringPiSetup ();
    flushed_print("Start Reading Test\n");

    // for polling 
    //ADXL355 adxl355(ADXL355::spi_channel,ADXL355::spi_speed,ADXL355::spi_mode,ADXL355::open_updateThread,ADXL355::polling_update_mode,ADXL355::isr_default);
    
    // for interrupt
    ADXL355::s_Init init_para;
    
    init_para.spi_channel = ADXL355::Default::spi_channel;
    init_para.spi_mode = ADXL355::Default::spi_mode;
    init_para.spi_speed = ADXL355::Default::spi_speed;
    init_para.updateMode = ADXL355::Default::INT_update_mode;
    init_para.updateThread = ADXL355::Default::open_updateThread;
    init_para.acc_range = ADXL355::Value::Range_4g;
    init_para.sampling_rate = ADXL355::Value::SamplingRate_4000;
    init_para.INT_pin = ADXL355::Default::INT1;
    init_para.isr_handler = irq_test_0;
    init_para.autoSetOffset = true;
    init_para.SetOffsetDataSize = ADXL355::Default::AVG_data_size;

    ADXL355 adxl355(init_para);

    ADXL355::AccUnit accunit;
    ADXL355::fAccUnit faccunit;
    struct timespec t_required, t_remain,t_main;
    double last_record_time = 0.0;
    double last_now = 0.0;

    ADXL355::fOffset foffset =  adxl355.readOffset();
    flushed_print("offset : {:6.5f}, {:6.5f}, {:6.5f}\n",foffset.fX,foffset.fY,foffset.fZ);

    adxl355.getAccRange();

    while(1)
    {
        
        if(!adxl355.dq_fAccUnitData.empty())
        {
            faccunit = adxl355.dq_pop_front();
            clock_gettime(CLOCK_REALTIME, &t_main);
            double now = time_diff_ms(&adxl355.adxl355_birth_time, &t_main);
            double ptime = now - faccunit.time_ms;

            if(ptime > 0.3)
            {
                flushed_print("now : {:6.3f}  (ms) |  record time : {:6.3f} (ms) | parse delay : {:6.3f} (ms) | record interval : {:6.3f} (ms)| x = {:6.3f} g | y = {:6.3f} g | z = {:6.3f} g | {}\n",
                  now,
                  faccunit.time_ms,
                  ptime,
                  faccunit.time_ms - last_record_time,
                  faccunit.fX,
                  faccunit.fY,
                  faccunit.fZ,
                  adxl355.dq_fAccUnitData.size()
                );   //flushed_print out test
            }
            
            if(now - last_now > 1000)
            {
                flushed_print("now : {:6.3f}  (s) | parse delay : {:6.3f} (ms) | record interval : {:6.3f} (ms)| x = {:6.3f} g | y = {:6.3f} g | z = {:6.3f} g | {}\n",
                now/1000,
                ptime,
                faccunit.time_ms - last_record_time,
                faccunit.fX,
                faccunit.fY,
                faccunit.fZ,
                adxl355.dq_fAccUnitData.size()
                );   //flushed_print out test
                last_now += 1000;
            }
            // assign last val
            last_record_time = faccunit.time_ms;
        }
    }
    flushed_print("Leaving ADXL main thread\n");
}