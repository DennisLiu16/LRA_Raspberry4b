#include <DRV2605L/LRA_DRV2605L.h>
using namespace LRA_DRV2605L;

/*(EN_pin,slav_id)*/
DRV2605L::DRV2605L(int EN_pin,int slave_id /*=SLAVE_DEFAULT_ID*/){

    this->slave_id = slave_id;
    this->EN_pin = EN_pin;
    /*class constructor*/
    this->data = Data();
}

DRV2605L::~DRV2605L(){
    if(is_init)
    {
        /*close bus*/
        stop();
        i2c_close(i2c.bus);
    }
}

void DRV2605L::init(){
    try{
        /*port open*/
        int tmp_port = -1;
        if( (tmp_port = i2c_open(I2C_DEFAULT_DEVICE)) == -1){
            /*port null*/
            throw ERR_DRV2605L_OPEN_PORT_FAILURE;
        }

        /*open successed, init i2c info*/
        i2c_init_device(&i2c,tmp_port,slave_id);
        pinMode(EN_pin,OUTPUT);
        digitalWrite(EN_pin, HIGH);
        is_init = 1;
    }
    catch(ErrorType e){
        errCode = e;
        format("{}\n",Error::getErrorName(e));
    }
}

void DRV2605L::set_LRA_6s()
{
    /*--seperate this later--*/
        //set_must(type)+set_custom()
    /*An example for how to set DRV2605L appropriately*/
    /*What you must setm if you don't know what value should it be, just set default*/
    /*Search should tune to find what you may need to adjust*/
    set_MODE(
        MODE_DEV_RESET_default |
        MODE_MODE_realtime_playback |        /*if you use pwm/analog -> C3*/
        MODE_STANDBY_standby 
    );
    set_LS(LS_HI_Z_default | LS_LIBRARY_SEL_lra);

    /*set Waveform sequencers empty*/
    for(uint8_t i = 0;i < WS_NUM;i++)
        set_WS(i,WS_WAIT_default | WS_WAV_FRM_SEQ_default);

    set_GO(GO_GO_stop);
    set_RV(RV_RATED_VOLTAGE_default);   /*should tune*/
    set_ODC(ODC_OD_CLAMP_default);      /*max voltage output*/  /*should tune*/
    set_FC(                             /*must set this before autocalibration*/ /*should tune*/
        FC_N_ERM_LRA_lra | 
        FC_FB_BRAKE_FACTOR_default |
        FC_LOOP_GAIN_default | 
        FC_BEMF_GAIN_default
    );
    set_C1(
        C1_STARTUP_BOOST_on |
        C1_AC_COUPLE_off |              /*this bit on only in analog input mode*/
        C1_DRIVE_TIME_default           /*should tune*/
    );
    set_C2(
        C2_BIDIR_INPUT_off |
        C2_BRAKE_STABILIZER_default |
        C2_SAMPLE_TIME_default |        /*should tune*/
        C2_BLANKING_TIME_lra_default |  /*should tune*/
        C2_IDISS_TIME_lra_default       /*should tune*/
    );
    set_C3(
        C3_DATA_FORMAT_RTP_signed |    /*use in pwm or analog*/ /*should tune*/
        C3_ERM_OPEN_LOOP_default |      /*ERM use*/
        C3_SUPPLY_COMP_DIS_enabled |    /*default enabled,if you have done this by yourself, turn off*/
        C3_DATA_FORMAT_RTP_default |    /*use in RTP mode*/
        C3_LRA_DRIVE_MODE_default |     /*LRA use, default once per cycle*/
        C3_N_PWM_ANALOG_default |       /*must set this bit if you use pwm/analog*/
        C3_LRA_OPEN_LOOP_default        /*auto closed loop*/
    );
    set_C4(
        C4_ZC_DET_TIME_default |
        C4_AUTO_CAL_TIME_500To700ms      /*should tune*/
    );
    set_C5(
        C5_AUTO_OL_CNT_default |
        C5_LRA_AUTO_OPEN_LOOP_never | 
        C5_PLAYBACK_INTERVAL_default |
        C5_BLANKING_TIME_lra_default |  /*LRA should tune*/
        C5_IDISS_TIME_lra_default       /*LRA should tune*/
    );    

    /*Optional*/
    set_RTP(RTP_RTP_INPUT_default); /*set this if you use RTP mode*/
    set_ODT(ODT_ODT_default);
    set_SPT(SPT_SPT_default);
    set_SNT(SNT_SNT_default);
    set_BRT(BRT_BRT_default);

    set_A2VC(A2VC_ATH_FILTER_default | A2VC_ATH_PEAK_TIME_default); /*set these if you use Audio Vibe mode*/
    set_A2VMaxIL(A2VMaxIL_ATH_MAX_INPUT_default);
    set_A2VMinIL(A2VMinIL_ATH_MIN_INPUT_default);
    set_A2VMaxOD(A2VMaxOD_ATH_MAX_DRIVE_default);
    set_A2VMinOD(A2VMinOD_ATH_MIN_DRIVE_default);

    set_ACCR(ACCR_A_CAL_COMP_default);  /*set this or not is ok, but if you don't set, plz use autocalibration mode to update these value first*/
    set_ACBR(ACBR_A_CAL_BEM_default);   /*same*/
    set_VVM(VVM_VBAT_default);
    set_LRARP(LRARP_LRA_PERIOD_default);

    set_LRAOLP(LRAOLP_OL_LRA_PERIOD_default);/*set this if you enable LRA open loop*/

    /*check*/
    print("set LRA done \n");
}

ssize_t DRV2605L::read(uint32_t reg_addr,void *buf,size_t len){
    /*addr range check*/
    try{
        /*equal to reg_addr>0 && reg_addr <= REG_MAX*/
        if( (reg_addr | REG_MAX - reg_addr) < 0){
            /*out of range*/
            format("Out of range. The error reg_addr is {:#04x}",reg_addr);
            throw ERR_DRV2605L_REGISTER_ADDRESS_DISMATCH;
        }

        /*send read request*/
        return i2c_read(&i2c,reg_addr,buf,len);;
    }
    catch(ErrorType e){
        errCode = e;
        format("{}\n",Error::getErrorName(e));
        return -1;
    }
}

void DRV2605L::print_all_register()
{
    /*print only useful registers*/
    uint8_t all[REG_NUM];
    i2c_read(&i2c,0x0,&all,REG_NUM);

    auto isSame = [](uint8_t a,uint8_t b){if(a!=b) return 'x';return ' ';};

    /*show in terminal*/
    int col_width = 20;
    /*print column's names , 15 char per column, fmt key - seperated*/
    print(emphasis::bold | fg(color::yellow),
        "{0:>{5}}{1:>{5}}{2:>{5}}{3:>{5}}{4:>{5}}{6:>{5}}\n",
        "Register_Addr",
        "Hex_Value",
        "Default_Hex",
        "Binary_Value",
        "Default_Binary",
        col_width,
        "isSame");

    for(uint8_t i = 0; i < REG_NUM; i++)
    {
        print( "{0:>{5}}{1:>{5}}{2:>{5}}{3:>{5}}{4:>{5}}{6}\n",
        format("{:#04x}",i),
        format("{:02x}",all[i]),
        format("{:02x}",Default_Value[i]),
        format("{:04b} {:04b}",all[i] >> 4,all[i] & 0b1111),
        format("{:04b} {:04b}",Default_Value[i] >> 4,Default_Value[i]& 0b1111),
        col_width,
        format(emphasis::bold | fg(color::yellow),"{0:>{1}}",isSame(all[i],Default_Value[i]), col_width)
        );
    }
    return;
}

uint8_t DRV2605L::read(uint32_t reg_addr)
{
    //bug need to assign a memory address to pointer alway, e.g. new uint8_t()
    uint8_t c = 0;
    read(reg_addr,&c,1);
    return c;
}

ssize_t DRV2605L::write(uint32_t reg_addr,const void* content, size_t len){
    try{
        /*equal to reg_addr>=1 && reg_addr <= REG_MAX*/
        if( ((reg_addr-1) | (REG_MAX - reg_addr)) < 0){
            /*out of range*/
            format("Out of range. The error reg_addr is {:#04x}",reg_addr);
            throw ERR_DRV2605L_REGISTER_ADDRESS_DISMATCH;
        }

        /*send write request*/
        return i2c_write(&i2c,reg_addr,content,len);
    }
    catch(ErrorType e){
        errCode = e;
        format("{}\n",Error::getErrorName(e));
        return -1;
    }
}

ssize_t DRV2605L::write(uint32_t reg_addr,uint8_t content)
{
    const uint8_t*c = &content;
    return write(reg_addr,c,1);
}

void DRV2605L::hard_reset()
{
    /*Write 0x01 with 0x80*/
    write(REG_Mode,MODE_DEV_RESET_reset);
}       

void DRV2605L::run()
{
    /*Set go bit,not valid for EN activate?*/
    /*get mode register*/
    uint8_t tmp = read(REG_Mode);
    tmp &= ~(1<<6);
    set(REG_Mode,MODE_STANDBY_ready|tmp);
    set(REG_Go,GO_GO_go);
}

void DRV2605L::stop()
{
    /*Cancel go bit, not valid for EN activate?*/
    uint8_t tmp = read(REG_Mode);
    tmp &= ~(1<<6);
    set(REG_Go,GO_GO_stop);
    set(REG_Mode,MODE_STANDBY_standby|tmp);
}   

void DRV2605L::run_autoCalibration()
{
    set_LRA_6s();
    run();
    sleep(1.5); // look at C4
    get_auto_calibration_info();
}

/**
 * @brief loop test
 * 
 */
void DRV2605L::run_RTPtest()
{
    set_LRA_6s();
    run();
    uint8_t amp = 0;
    bool flag = 0;
    int loop = -1;
    /*Loop test*/
    while(loop <= 1)
    {
        // if(!flag)
        //     amp++;
        // else    
        //     amp--;

        // if(amp >= 255)
        //     flag = 1;
        // else if(amp == 0)
        // {   flag = 0;
        //     loop++;
        // }
            
        set_amplitude(152);
        //usleep(2e4);
    }
    stop();
}

/*Set function*/
inline void DRV2605L::set_amplitude(uint8_t amp)
{
    set_RTP(amp);
    print("LRA resonance frequency : {:.3f} Hz , amp = {}\n",get_operating_hz(),amp);
}

/*Get function*/
inline uint8_t DRV2605L::get_ACCR()
{
    /*Get Auto-Calibration Compensation Result*/
    return read(REG_AutoCalibrationCompensationResult);
}          
inline uint8_t DRV2605L::get_ACBR()
{
    /*Get Auto-Calibration Back-EMF Result*/
    return read(REG_AutoCalibrationBackEMFResult);

}          
inline uint8_t DRV2605L::get_VVM()
{
    /*Get Vbat Voltage Monitor*/
    return read(REG_VbatVoltageMonitor);
}           
inline double DRV2605L::get_operating_hz()
{
    /*Get LRA Resonance Period*/
    return 1e6/((double)read(REG_LRAResonancePeriod)*98.46);
}

void DRV2605L::get_auto_calibration_info()
{
    if( (read(REG_Status) & (1<<3)) != 0 )
    {
        print("Auto calibration failed\n");
    }
    else
    {
        print("--------------------------\n");
        print("Auto Calibration successed\n");
        print("LRA resonance frequency : {:.3f} Hz\n",get_operating_hz());
        print("Leaving Auto Calibration Mode\n");
    }
        
}

/*------------------------------------------Protected-----------------------------------------------*/

void DRV2605L::set_MODE(uint8_t content)
{
    /*Mode*/
    set(REG_Mode,content);
} 
void DRV2605L::set_RTP(uint8_t content)
{
    /*Real-Time Playback Input*/
    set(REG_RealTimePlaybackInput,content);
}
void DRV2605L::set_LS(uint8_t content)
{
    /*Library Selection*/
    set(REG_LibrarySelection,content);
}
void DRV2605L::set_WS(uint8_t num,uint8_t content)
{
    /*Waveform Sequencer, from num 0 to num 7*/
    set(REG_WaveformSequencer_Head+num,content);
}
void DRV2605L::set_ODT(uint8_t content)
{
    /*Overdrive Time Offset*/
    set(REG_OverdriveTimeOffset,content);
}
void DRV2605L::set_SPT(uint8_t content)
{
    /*Sustain Time Positive Offset*/
    set(REG_SustainTimeOffsetPositive,content);
}       
void DRV2605L::set_SNT(uint8_t content)
{
    /*Sustain Time Negative Offset*/
    set(REG_SustainTimeOffsetNegative,content);
}       
void DRV2605L::set_BRT(uint8_t content)
{
    /*Brake Time*/
    set(REG_BrakeTimeOffset,content);
}       
void DRV2605L::set_A2VC(uint8_t content)
{
    /*Audio-to-Vibe Control*/
    set(REG_Audio2VibeControl,content);
}      
void DRV2605L::set_A2VMinIL(uint8_t content)
{
    /*Audio-to-Vibe Minimum Input Level*/
    set(REG_Audio2VibeMinimumInputLevel,content);
}  
void DRV2605L::set_A2VMaxIL(uint8_t content)
{
    /*Audio-to-Vibe Maximum Input Level*/
    set(REG_Audio2VibeMaximumIuputLevel,content);
}   
void DRV2605L::set_A2VMinOD(uint8_t content)
{
    /*Audio-to-Vibe Minimum Output Drive*/
    set(REG_Audio2VibeMinimumOuputDrive,content);
}  
void DRV2605L::set_A2VMaxOD(uint8_t content)
{
    /*Audio-to-Vibe Maximum Output Drive*/
    set(REG_Audio2VibeMaximumOuputDrive,content);
}   
void DRV2605L::set_RV(uint8_t content)
{
    /*Rated Voltage -> use in auto calibration*/
    set(REG_RatedVoltage,content);
}        
void DRV2605L::set_ODC(uint8_t content)
{
    /*Overdrive Clamp Voltage*/
    set(REG_OverdriveClampVoltage,content);
}       
void DRV2605L::set_FC(uint8_t content)
{
    /*Feedback Control*/
    set(REG_FeedbackControl,content);
}        
void DRV2605L::set_C1(uint8_t content)
{
    /*Control 1*/
    set(REG_Control1,content);
}        
void DRV2605L::set_C2(uint8_t content)
{
    /*Control 2*/
    set(REG_Control2,content);
}        
void DRV2605L::set_C3(uint8_t content)
{
    /*Control 3*/
    set(REG_Control3,content);
}        
void DRV2605L::set_C4(uint8_t content)
{
    /*Control 4*/
    set(REG_Control4,content);
}        
void DRV2605L::set_C5(uint8_t content)
{
    /*Control 5*/
    set(REG_Control5,content);
}        
void DRV2605L::set_LRAOLP(uint8_t content)
{
    /*LRA open loop period*/
    set(REG_LRAOpenLoopPeriod,content);
}    
void DRV2605L::set_LRARP(uint8_t content)
{
    /*LRA resonance period*/
    set(REG_LRAResonancePeriod,content);
}     
void DRV2605L::set_GO(uint8_t content)
{
    /*Go*/
    set(REG_Go,content);
}
void DRV2605L::set_ACCR(uint8_t content)
{
    set(REG_AutoCalibrationCompensationResult,content);
}
void DRV2605L::set_ACBR(uint8_t content)
{
    set(REG_AutoCalibrationBackEMFResult,content);
}
void DRV2605L::set_VVM(uint8_t content)
{
    set(REG_VbatVoltageMonitor,content);
}


/*--------------------------Private---------------------------*/
void DRV2605L::info(uint32_t reg_addr,uint8_t content)
{
    print("Register address is {0} : {1} {2}, hex : {3}\n",
        format(emphasis::bold | fg(color::cyan),"{:#04x}",reg_addr),
        format(emphasis::bold | fg(color::red),"{:04b}",(content>>4)),
        format(emphasis::bold | fg(color::yellow),"{:04b}",(0b00001111)&content),
        format(emphasis::bold | fg(color::green),"{:02x}",content)
    );
}

void DRV2605L::set(uint32_t reg_addr,uint8_t content)
{
    //uint8_t need_to_change = content | read(reg_addr);    //bug 
    //content |= read(reg_addr);
    //info(reg_addr,content);
    write(reg_addr,content);
}


