/**
 * @file DRV2605L_TCA.cpp
 * @author Dennis Liu (liusx880630@gmail.com)
 * @brief For mutiple DRV2605L with TCA9548A I2C multiplexer
 * @version 1.0
 * @date 2022-01-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <DRV2605L_TCA/DRV2605L_TCA.h>

// class alias
using namespace LRA_DRV2605L_TCA;
using DRV = DRV2605L_TCA;

// static init
int DRV::_drvNum = 0;
int DRV::_bus = -1;
uint8_t DRV::_controlReg = 0;
I2CDevice DRV::i2c_TCA;

//////////
/*public*/
//////////
DRV::DRV2605L_TCA()
{
    /*expand yourself*/
}

DRV::DRV2605L_TCA(int EN, int channel)
{
    if(channel > 7)
    {
        perror("channel from 0 to 7");
        exit(EXIT_FAILURE);
    }
    
    _EN = EN;
    _channel = channel;
    _thisControlReg = 1 << _channel;

    // bus init
    if(_bus <= 0)
    {
        try{
            /*bus open*/
            int tmp_bus = -1;
            if( (tmp_bus = i2c_open(I2C_DEFAULT_DEVICE)) == -1){
                /*port null*/
                throw ERR_DRV2605L_OPEN_PORT_FAILURE;
            }
            _bus = tmp_bus;
        }
        catch(ErrorType e)
        {
            flushed_print("{}\n",Error::getErrorName(e));
        }
    }

    // init TCA bus
    if(i2c_TCA.bus == 0)
        i2c_init_device(&i2c_TCA, _bus, Default::TCA_SLAVE_ID);
    
    // init DRV
    i2c_init_device(&i2c_DRV, _bus, Default::DRV_SLAVE_ID);
    pinMode(_EN,OUTPUT);
    digitalWrite(_EN, HIGH);

    ++_drvNum;
}

DRV::~DRV2605L_TCA()
{
    --_drvNum;
    if(_drvNum == 0 && _bus > 0)
        i2c_close(_bus);
}

void 
DRV::TCA_selectMultiChannel(uint8_t controlReg/* ch should be 8 bits to present 8 channels' state*/)
{
    /*is same*/
    if(controlReg == _controlReg)
        return;

    if(i2c_write(&i2c_TCA, Default::TCA_REGADDR, &controlReg, 1))
    {
        /*update _controlReg*/
        _controlReg = controlReg;
        return;
    }

    flushed_print("TCA_selectMultiChannel failed\n");
}

void 
DRV::TCA_selectSingleChannel(uint8_t ch)
{
    if(1<<ch == _controlReg)
        return;

    uint8_t c = 1 << ch;
    
    if(i2c_write(&i2c_TCA, Default::TCA_REGADDR, &c, 1))
    {
        /*update _controlReg*/
        _controlReg = c;
        return;
    }
}

ssize_t 
DRV::multiRegWrite(uint8_t regAddr, const void* content, size_t len)
{
    try{
        /*equal to reg_addr>=1 && reg_addr <= REG_MAX*/
        if( ((regAddr-1) | (AddrMax - regAddr)) < 0){
            /*out of range*/
            flushed_print("Out of range. The error reg_addr is {:#04x}",regAddr);
            throw ERR_DRV2605L_REGISTER_ADDRESS_DISMATCH;
        }

        /*send write request*/
        return i2c_write(&i2c_DRV,regAddr,content,len);
    }
    catch(ErrorType e){
        flushed_print("{}\n",Error::getErrorName(e));
        return -1;
    }
}

ssize_t 
DRV::multiRegWrite(uint8_t regAddr, const void* content, size_t len, uint8_t controlReg)
{
    /*change control register of TCA*/
    if(_controlReg != controlReg)
        TCA_selectMultiChannel(controlReg);
    
    return multiRegWrite(regAddr, content, len);
}

ssize_t 
DRV::multiRegRead(uint8_t regAddr, void* buf, size_t len)
{
    try{
        /*equal to reg_addr>=0 && reg_addr <= REG_MAX*/
        if( (regAddr | (AddrMax - regAddr)) < 0){
            /*out of range*/
            flushed_print("Out of range. The error reg_addr is {:#04x}",regAddr);
            throw ERR_DRV2605L_REGISTER_ADDRESS_DISMATCH;
        }

        /*send read request*/
        return i2c_read(&i2c_DRV,regAddr,buf,len);
    }
    catch(ErrorType e){
        flushed_print("{}\n",Error::getErrorName(e));
        return -1;
    }
}

ssize_t
DRV::multiRegRead(uint8_t regAddr, void* buf, size_t len, uint8_t controlReg)
{
    /*change control register of TCA*/
    if(_controlReg != controlReg)
        TCA_selectMultiChannel(controlReg);
    
    return multiRegRead(regAddr, buf, len);
}

void 
DRV::setBitPair(int regIndex, uint8_t val)
{
    /*range check*/
    if((regIndex | (IndexMax - regIndex)) < 0)
    {
        flushed_print("RegIndex out of range -- setBitPair\n");
        return;
    }
    /*if length == 8, set directly*/
    uint8_t c;
    if(length[regIndex] == 8)
        c = val;

    /*if length < 8, get the register value first*/
    else{
        if(multiRegRead(addr[regIndex], &c, 1) < 0)
        {
            flushed_print("read reg failed -- setBitPair\n");
            return;
        }

        /*mask*/
        uint8_t writeMask = getmask(length[regIndex], startbit[regIndex]);
        uint8_t restVal = c & (~writeMask);
        uint8_t setVal = (val << startbit[regIndex]) & writeMask;
        c = restVal | setVal;
    }
    /*setBitPair val*/
    multiRegWrite(addr[regIndex], &c, 1);
}

void 
DRV::setBitPair(int regIndex, uint8_t val, uint8_t controlReg)
{
    if(_controlReg != controlReg)
        TCA_selectMultiChannel(controlReg);
    
    return setBitPair(regIndex, val);
}

uint8_t 
DRV::getBitPair(int regIndex)
{
    if((regIndex | (IndexMax - regIndex)) < 0)
    {
        flushed_print("RegIndex out of range -- getBitPair\n");
        return 0;
    }
    uint8_t buf;
    if(multiRegRead(addr[regIndex], &buf, 1) < 0)
        flushed_print("getBitPair failed \n");
    
    /*if bitPair = whole register*/
    if(length[regIndex] == 8)
        return buf;

    uint8_t bitPairVal = buf & getmask(length[regIndex], startbit[regIndex]);
    return (bitPairVal >> startbit[regIndex]);
}

uint8_t
DRV::getBitPair(int regIndex, uint8_t controlReg)
{
    if(_controlReg != controlReg)
        TCA_selectMultiChannel(controlReg);
    
    return getBitPair(regIndex);
}

// Normal DRV functions
void 
DRV::reset()
{
    flushed_print("No.{} device reset cmd\n", _channel);
    setBitPair(regIndex::DEV_RESET, DEV_RESET_reset,  _thisControlReg);
    sleep(1);
}

void
DRV::setRTP(uint8_t val)
{
    setBitPair(regIndex::RTP_INPUT, val, _thisControlReg);
}

void
DRV::setGo(bool flag)
{
    if(flag)
    {
        setBitPair(regIndex::GO, GO_go, _thisControlReg);
        return;
    }
    setBitPair(regIndex::GO, GO_stop, _thisControlReg);
}

void 
DRV::setStandBy(bool flag)
{
    if(flag == STANDBY_ready)
    {
        setBitPair(STANDBY,STANDBY_ready);
        return;
    }
     setBitPair(STANDBY,STANDBY_standby);
}

double 
DRV::getOperationFreq()
{
    uint8_t val = getBitPair(regIndex::LRA_PERIOD, _thisControlReg);
    return  1 / ((double)val * unit.LRA_PERIOD_us) * 1e6;
}

void 
DRV::getStatusInfo()
{
    uint8_t buf;
    if(multiRegRead(Addr::Status, &buf, 1) < 0)
    {
        flushed_print("getStatusInfo failed\n");
        return;
    }

    uint8_t device_id = (buf & getmask(length[DEVICE_ID],startbit[DEVICE_ID])) >> startbit[DEVICE_ID];
    uint8_t diag_result = (buf & getmask(length[DIAG_RESULT],startbit[DIAG_RESULT])) >> startbit[DIAG_RESULT];
    uint8_t over_temp = (buf & getmask(length[OVER_TEMP],startbit[OVER_TEMP])) >> startbit[OVER_TEMP];
    uint8_t oc_detect = (buf & getmask(length[OC_DETECT],startbit[OC_DETECT])) >> startbit[OC_DETECT];

    switch(device_id)
    {
        case 3: 
            flushed_print("The device is : DRV2605 (contains licensed ROM library, does not contain RAM)\n");
            break;
        case 4:
            flushed_print("The device is : DRV2604 (contains RAM, does not contain licensed ROM library)\n");
            break;
        case 6:
            flushed_print("The device is : DRV2604L (low-voltage version of the DRV2604 device)\n");
            break;
        case 7:
            flushed_print("The device is : DRV2605L (low-voltage version of the DRV2605 device)\n");
            break;
    }

    std::string str = (!diag_result) ? "Diag/calibration : Passed\n" : "Diag/calibration : Failed\n";
    flushed_print("{}\n",str);

    str = (!over_temp) ? "Device overtemp\n" : "Temp normal\n";
    flushed_print("{}\n",str);

    str = (!oc_detect) ? "Overcurrent detected\n" : "Current noraml\n";
    flushed_print("{}\n",str);
}

double
DRV::getBEMFgain()
{
    uint8_t val = getBitPair(BEMF_GAIN);

    bool flag = (getBitPair(N_ERM_LRA) == N_ERM_LRA_lra);

    switch(val)
    {
        // flag ? LRA : ERM
        case BEMF_GAIN_low:
            return (flag) ? 3.75 : 0.255;
        case BEMF_GAIN_medium:
            return (flag) ? 7.5 : 0.7875;
        case BEMF_GAIN_high:
            return (flag) ? 15.0 : 1.365;
        case BEMF_GAIN_veryhi:
            return (flag) ? 22.5 : 3.0;
        default:
            return 0.0;
    }
}

double 
DRV::getPlayBackInterval()
{
    return getBitPair(PLAYBACK_INTERVAL) ? 1.0 : 5.0;
}

double 
DRV::getLRASampleTime()
{
    switch(getBitPair(SAMPLE_TIME))
    {
        case SAMPLE_TIME_150us:
            return 150.0;
        case SAMPLE_TIME_200us:
            return 200.0;
        case SAMPLE_TIME_250us:
            return 250.0;
        case SAMPLE_TIME_300us:
            return 300.0;
        default:
            return 0.0;
    }
}

double 
DRV::getDriveTime()
{
    bool mode = getBitPair(N_ERM_LRA);
    uint8_t t_drive = getBitPair(DRIVE_TIME);
    if(mode == N_ERM_LRA_erm)
        return t_drive * 0.2 + 1;
    
    return t_drive * 0.1 + 0.5; 
}

double
DRV::getAVorRMSVoltage()
{
    /*do it later*/
    bool mode = getBitPair(N_ERM_LRA);
    return 1.0;
}

double 
DRV::getOverDriveClampVoltage()
{
    bool mode = getBitPair(N_ERM_LRA);
    uint8_t od_clamp = getBitPair(OD_CLAMP);
    if(mode == N_ERM_LRA_erm)
    {
        uint8_t t_drive = getDriveTime();
        uint8_t t_idiss = getIdissTime();
        uint8_t t_blank = getBlankingTime();
        return 21.64*1e-3*od_clamp*(t_drive*1e-3 - 300e-6)/(t_drive*1e-3 + t_idiss*1e-6 + t_blank*1e-6);
    }
    return 21.22e-3*od_clamp;
}

double
DRV::getBlankingTime()
{
    /*mode*/
    bool mode = getBitPair(N_ERM_LRA);
    uint8_t bt0 = getBitPair(BLANKING_TIME0);
    uint8_t bt1 = getBitPair(BLANKING_TIME1);
    uint8_t bt = (bt1 << length[BLANKING_TIME0] | bt0);
    if(mode == N_ERM_LRA_erm)
    {
        switch(bt)
        {
            case BLANKING_TIME_erm_45us:
                return 45.0;
            case BLANKING_TIME_erm_75us:
                return 75.0;
            case BLANKING_TIME_erm_150us:
                return 150.0;
            case BLANKING_TIME0_erm_225us:
                return 225.0;
            default:
                return 0.0;
        }
    }
    else
    {
        switch(bt)
        {
            case BLANKING_TIME_lra_15us:
                return 15.0;
            case BLANKING_TIME_lra_25us:
                return 25.0;
            case BLANKING_TIME_lra_50us:
                return 50.0;
            case BLANKING_TIME_lra_75us:
                return 75.0;
            case BLANKING_TIME_lra_90us:
                return 90.0;
            case BLANKING_TIME_lra_105us:
                return 105.0;
            case BLANKING_TIME_lra_120us:
                return 120.0;
            case BLANKING_TIME_lra_135us:
                return 135.0;
            case BLANKING_TIME_lra_150us:
                return 150.0;
            case BLANKING_TIME_lra_165us:
                return 165.0;
            case BLANKING_TIME_lra_180us:
                return 180.0;
            case BLANKING_TIME_lra_195us:
                return 195.0;
            case BLANKING_TIME_lra_210us:
                return 210.0;
            case BLANKING_TIME_lra_235us:
                return 235.0;
            case BLANKING_TIME_lra_260us:
                return 260.0;
            case BLANKING_TIME_lra_285us:
                return 285.0;
            default:
                return 0.0;
        }
    }
}

double 
DRV::getIdissTime()
{
    bool mode = getBitPair(N_ERM_LRA);
    uint8_t is0 = getBitPair(IDISS_TIME0);
    uint8_t is1 = getBitPair(IDISS_TIME1);
    uint8_t is = is1 << length[IDISS_TIME0] | is0;

    if(mode == N_ERM_LRA_erm)
    {
        switch(is)
        {
            case IDISS_TIME_erm_45us:
                return 45.0;
            case IDISS_TIME_erm_75us:
                return 75.0;
            case IDISS_TIME_erm_150us:
                return 150.0;
            case IDISS_TIME0_erm_225us:
                return 225.0;
            default:
                return 0.0;
        }
    }
    else
    {
        switch(is)
        {
            case IDISS_TIME_lra_15us:
                return 15.0;
            case IDISS_TIME_lra_25us:
                return 25.0;
            case IDISS_TIME_lra_50us:
                return 50.0;
            case IDISS_TIME_lra_75us:
                return 75.0;
            case IDISS_TIME_lra_90us:
                return 90.0;
            case IDISS_TIME_lra_105us:
                return 105.0;
            case IDISS_TIME_lra_120us:
                return 120.0;
            case IDISS_TIME_lra_135us:
                return 135.0;
            case IDISS_TIME_lra_150us:
                return 150.0;
            case IDISS_TIME_lra_165us:
                return 165.0;
            case IDISS_TIME_lra_180us:
                return 180.0;
            case IDISS_TIME_lra_195us:
                return 195.0;
            case IDISS_TIME_lra_210us:
                return 210.0;
            case IDISS_TIME_lra_235us:
                return 235.0;
            case IDISS_TIME_lra_260us:
                return 260.0;
            case IDISS_TIME_lra_285us:
                return 285.0;
            default:
                return 0.0;
        }
    } 
}

void 
DRV::printAllRegIndex()
{   string s = format("|{:^20} | {:^4} | {:^5} : {}","regIndex name","dec","hex","description|");
    string line = format("{0:-^{1}}","",s.size());
    flushed_print("{0:^}\n{1}\n{0:^}\n",line,s);
    for(int index = 0; index < NUM_REG; ++index)
    {
        uint8_t val = getBitPair(index);
        
        switchInfo(index, val);
    }
    flushed_print("\n\n");
}

void
DRV::switchInfo(int index, uint8_t val)
{
    /*only useful information show*/
    std::string str;
    switch(index)
    {
        case DEVICE_ID:
            switch(val)
            {
                case 3: 
                    str = "Device is DRV2605 (contains licensed ROM library, does not contain RAM)";
                    break;
                case 4:
                    str = "Device is DRV2604 (contains RAM, does not contain licensed ROM library)";
                    break;
                case 6:
                    str = "Device is DRV2604L (low-voltage version of the DRV2604 device)";
                    break;
                case 7:
                    str = "Device is DRV2605L (low-voltage version of the DRV2605 device)";
                    break;
            }
            break;
        case DIAG_RESULT:
            str = (!val) ? "Passed" : "Failed";
            break;
        case OVER_TEMP:
            str = (val) ? "Over temperature" : "Nomral temperature";
            break;
        case OC_DETECT:
            str = (val) ? "Over current" : "Normal current";
            break;
        case DEV_RESET:
            str = "Normal";
            break;
        case STANDBY:
            str = (!val) ? "Device ready" : "Device standby";
            break;
        case MODE:
            switch(val)
            {
                case MODE_internalTrigger:
                    str = "Internal trigger";
                    break;
                case MODE_external_trigger_edge:
                    str = "External trigger (edge mode)";
                    break;
                case MODE_external_trigger_level:
                    str = "External trigger (level mode)";
                    break;
                case MODE_pwm_or_analog:
                    str = "PWM input and analog input";
                    break;
                case MODE_audio2vibe:
                    str = "Audio-to-vibe";
                    break;
                case MODE_realtime_playback:
                    str = "Real-time playback (RTP mode)";
                    break;
                case MODE_diagnostics:
                    str = "Diagnostics";
                    break;
                case MODE_auto_calibration:
                    str = "Auto calibration";
                    break;
            }
            break;
        case HI_Z:
            str = (val) ? "HI Z (15k ohm)" : "Low Z (default)";
            break;
        case LIBRARY_SEL:
            switch(val)
            {
                case LIBRARY_SEL_empty :
                    str =  "Empty";
                    break;
                case LIBRARY_SEL_a:
                    str = "TS2200 Library A";
                    break;
                case LIBRARY_SEL_b:
                    str = "TS2200 Library B";
                    break;
                case LIBRARY_SEL_c:
                    str = "TS2200 Library C";
                    break;
                case LIBRARY_SEL_d:
                    str = "TS2200 Library D";
                    break;
                case LIBRARY_SEL_e:
                    str = "TS2200 Library E";
                    break;
                case LIBRARY_SEL_f:
                    str = "TS2200 Library F";
                    break;
                case LIBRARY_SEL_lra:
                    str = "LRA Library";
                    break;
            }
            break;
        case GO:
            str = (val) ? "Go" : "Stop";
            break;
        case ODT:
            str = format("Overdrive Time Offset = {:.3f} (ms)", val*getPlayBackInterval());
            break;
        case SPT:
            str = format("Sustain-Time Positive Offset = {:.3f} (ms)", val*getPlayBackInterval());
            break;
        case SNT:
            str = format("Sustain-Time Negative Offset = {:.3f} (ms)", val*getPlayBackInterval());
            break;
        case BRT:
            str = format("Brake Time Offset = {:.3f} (ms)", val*getPlayBackInterval());
            break;
        case ATH_PEAK_TIME:
            switch(val)
            {
                case 0:
                    str = "Peak detection time = 10ms, for mode audio-to-vibe";
                    break;
                case 1:
                    str = "Peak detection time = 20ms, for mode audio-to-vibe";
                    break;
                case 2:
                    str = "Peak detection time = 30ms, for mode audio-to-vibe";
                    break;
                case 3:
                    str = "Peak detection time = 40ms, for mode audio-to-vibe";
                    break;
            }
            break;
        case ATH_FILTER:
            switch(val)
            {
                case 0:
                    str = "Low-pass filter frequency 100 Hz for mode audio-to-vibe";
                    break;
                case 1:
                    str = "Low-pass filter frequency 125 Hz for mode audio-to-vibe";
                    break;
                case 2:
                    str = "Low-pass filter frequency 150 Hz for mode audio-to-vibe";
                    break;
                case 3:
                    str = "Low-pass filter frequency 200 Hz for mode audio-to-vibe";
                    break;
            }
            break;
        case ATH_MIN_INPUT:
            str = format("Audio-to-vibe min input Voltage (VPP) = {:.3f} (V)", val*unit.ATH_INPUT_V/255);
            break;
        case ATH_MAX_INPUT:
            str = format("Audio-to-vibe max input Voltage (VPP) = {:.3f} (V)", val*unit.ATH_INPUT_V/255);
            break;
        case ATH_MIN_DRIVE:
            str = format("Audio-to-vibe min output level = {:.3f} (%)", static_cast<double>(val)/255*100);
            break;
        case ATH_MAX_DRIVE:
            str = format("Audio-to-vibe max output level = {:.3f} (%)", static_cast<double>(val)/255*100);
            break;
        case A_CAL_COMP:
            str = format("Auto calibration result coefficient = {:.3f}", 1+static_cast<double>(val)/255);
            break;
        case A_CAL_BEMF:
            str = format("Auto-calibration back-EMF (V) = {:.3f}", static_cast<double>(val)/255*unit.AUTO_CALIBRATION_BACK_EMF_V/getBEMFgain());
            break;
        case N_ERM_LRA:
            str = (val) ? "LRA mode" : "ERM mode";
            break;
        case FB_BRAKE_FACTOR:
            switch(val)
            {
                case FB_BRAKE_FACTOR_1x:
                    str = "1x";
                    break;
                case FB_BRAKE_FACTOR_2x:
                    str = "2x";
                    break;
                case FB_BRAKE_FACTOR_3x:
                    str = "3x";
                    break;
                case FB_BRAKE_FACTOR_4x:
                    str = "4x";
                    break;
                case FB_BRAKE_FACTOR_6x:
                    str = "6x";
                    break;
                case FB_BRAKE_FACTOR_8x:
                    str = "8x";
                    break;
                case FB_BRAKE_FACTOR_16x:
                    str = "16x";
                    break;
                case FB_BRAKE_FACTOR_disabled:
                    str = "disabled";
                    break;  
            }
            break;
        case LOOP_GAIN:
            switch(val)
            {
                case LOOP_GAIN_low:
                    str = "Low";
                    break;
                case LOOP_GAIN_medium:
                    str = "Medium";
                    break;
                case LOOP_GAIN_high:
                    str = "High";
                    break;
                case LOOP_GAIN_veryhi:
                    str = "Very high";
                    break;
            }
            break;
        case BEMF_GAIN:
            str = format("BEMF gain = {:.4f}x", getBEMFgain());
            break;
        case STARTUP_BOOST:
            str = (val) ? "On": "Off";
            break;
        case AC_COUPLE:
            str = (val) ? "AC coupling" : "DC coupling";
            break;
        case DRIVE_TIME:
        {
            bool flag = getBitPair(N_ERM_LRA);
            double drive_time = flag ? val*unit.DRIVE_TIME_LRA_ms+bias.DRIVE_TIME_LRA_ms : val*unit.DRIVE_TIME_ERM_ms+bias.DRIVE_TIME_ERM_ms;
            str = format("Drive time = {:.3f} (ms)",drive_time);
            break;
        }
        case BIDIR_INPUT:
            str = (val) ? "Bidirectional input mode (default)" : "Unidirectional input mode";
            break;
        case BRAKE_STABILIZER:
            str = (val) ? "On" : "Off";
            break;
        case SAMPLE_TIME:
            str = format("LRA auto-resonance sampling time = {:.1f} (us)", getLRASampleTime());
            break;
        case BLANKING_TIME0:
            str = format("Blanking time = {:.1f} (us)", getBlankingTime());
            break;
        case IDISS_TIME0:
            str = format("Idmiss time = {:.1f} (us)", getIdissTime());
        case NG_THRESH:
            switch(val)
            {
                case NG_THRESH_disabled:
                    str = "Noise-gate threshold for PWM or analog input disabled"; 
                    break;
                case NG_THRESH_2pct:
                    str = "Noise-gate threshold for PWM or analog input = 2%"; 
                    break;
                case NG_THRESH_4pct:
                    str = "Noise-gate threshold for PWM or analog input = 4%"; 
                    break;
                case NG_THRESH_8pct:
                    str = "Noise-gate threshold for PWM or analog input = 8%"; 
                    break;

            }
            break;
        case ERM_OPEN_LOOP:
            str = (val) ? "Open loop" : "Closed loop";
            break;
        case SUPPLY_COMP_DIS:
            str = (val) ? "Supply compensation disabled" : "Supply compensation enabled";
            break;
        case DATA_FORMAT_RTP:
            str = (val) ? "Unsigned" : "Signed";
            break;
        case LRA_DRIVE_MODE:
            str = (val) ? "Twice per cycle" : "Once per cycle";
            break;
        case N_PWM_ANALOG:
            str = (val) ? "Analog Input" : "PWM Input";
            break;
        case LRA_OPEN_LOOP:
            str = (val) ? "LRA open-loop mode" : "Auto-resonance mode";
            break;
        case ZC_DET_TIME:
            switch(val)
            {
                case ZC_DET_TIME_100us:
                    str = "Zero crossing minimum length of time = 100 (us)";
                    break;
                case ZC_DET_TIME_200us:
                    str = "Zero crossing minimum length of time = 200 (us)";
                    break;
                case ZC_DET_TIME_300us:
                    str = "Zero crossing minimum length of time = 300 (us)";
                    break;
                case ZC_DET_TIME_390us:
                    str = "Zero crossing minimum length of time = 390 (us)";
                    break;
            }
            break;
        case AUTO_CAL_TIME:
            switch(val)
            {
                case AUTO_CAL_TIME_150To350ms:
                    str = "Auto caibration time from 150 to 350 (ms)";
                    break;
                case AUTO_CAL_TIME_250To450ms:
                    str = "Auto caibration time from 250 to 450 (ms)";
                    break;
                case AUTO_CAL_TIME_500To700ms:
                    str = "Auto caibration time from 500 to 700 (ms)";
                    break;
                case AUTO_CAL_TIME_1000To1200ms:
                    str = "Auto caibration time from 1000 to 1200 (ms)";
                    break;
            }
            break;
        case OTP_STATUS:
            str = (val) ? "OTP Memory has been programmed" : "OTP Memory has not been programmed";
            break;
        case AUTO_OL_CNT:
            switch(val)
            {
                case AUTO_OL_CNT_3:
                    str = "3 attempts";
                    break;
                case AUTO_OL_CNT_4:
                    str = "4 attempts";
                    break;
                case AUTO_OL_CNT_5:
                    str = "5 attempts";
                    break;
                case AUTO_OL_CNT_6:
                    str = "6 attempts";
                    break;
            }
            break;
        case LRA_AUTO_OPEN_LOOP:
            str = (val) ? "Automatically transitions to open loop" : "Never transitions to open loop";
            break;
        case PLAYBACK_INTERVAL:
            str = (val) ? "1(ms)" : "5(ms)";
            break;
        case OL_LRA_PERIOD:
            str = format("LRA open-loop period = {:.2f} (us)", val*unit.LRA_PERIOD_us);
            break;
        case VBAT:
            str = format("V(BAT) Voltage(VDD) = {:.3f} (V)", val*unit.VBAT_V/255);
            break;
        case LRA_PERIOD:
            str = format("LRA Period = {:.3f} (us) = {:.3f}(Hz)", val*unit.LRA_PERIOD_us, 1/(unit.LRA_PERIOD_us*val)*1e6);
            break;
        default:
            str = "";
            break;
    }
    flushed_print("|{:^20} | {:^4} | {:^#5X} : {}\n",regName[index], val, val, str);
    flushed_print("|{:-^20} | {:-^4} | {:-^5}   {}\n","", "", "", "");
}

// LRA project setting 
void 
DRV::set6S()
{
    /*TCA check*/
    TCA_selectSingleChannel(_channel);

    /*Mode*/
    setBitPair(MODE, MODE_realtime_playback);

    /*RTP set to 0*/
    setBitPair(RTP_INPUT, 0);

    /*Library Selection*/
    setBitPair(LIBRARY_SEL, LIBRARY_SEL_lra);

    /*Feedback Control*/
    setBitPair(N_ERM_LRA, N_ERM_LRA_lra);

    /*Control 1*/
    setBitPair(STARTUP_BOOST, STARTUP_BOOST_on);
    setBitPair(AC_COUPLE, AC_COUPLE_off);

    /*Control 2*/
    setBitPair(BIDIR_INPUT, BIDIR_INPUT_off);

    /*Control 3*/
    setBitPair(DATA_FORMAT_RTP, DATA_FORMAT_RTP_signed);
    setBitPair(SUPPLY_COMP_DIS, SUPPLY_COMP_DIS_enabled);

    /*Control 4*/
    setBitPair(AUTO_CAL_TIME, AUTO_CAL_TIME_500To700ms);

    /*Control 5*/
    setBitPair(LRA_AUTO_OPEN_LOOP, LRA_AUTO_OPEN_LOOP_never);

    /*Custom*/

    flushed_print("set 6S done \n");
}

/////////////
/*protected*/
/////////////



///////////
/*private*/
///////////



