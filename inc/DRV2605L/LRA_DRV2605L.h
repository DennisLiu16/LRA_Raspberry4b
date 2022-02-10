/** @file         DRV2605L.h
 *  @brief        簡要說明
 *  @details      詳細說明
 *  @author       Dennis
 *  @date         2021-11-07 
 *  @version      v1.0
 *  @copyright    Copyright By Dennis, All Rights Reserved
 *
 **********************************************************
 *  @LOG:
 **********************************************************
*/

#ifndef _LRA_DRV2605L_H_
#define _LRA_DRV2605L_H_
#include <I2c/i2c.h>
#include <ErrorCode/LRA_ErrorCode.h>
#include <PI/LRA_PI_Util.h>
extern "C" {
#include <wiringPi.h>
#include <signal.h>
}


namespace LRA_DRV2605L{
    using namespace LRA_PI_Util;
    using namespace LRA_Error;
     
    /*Slave id (i2c address)*/
    typedef enum{
        SLAVE_DEFAULT_ID = 0x5a,
        /*after get tca9548a will use these slave id*/
        SLAVE_X_ID       = 0x71,
        SLAVE_Y_ID       = 0x72,
        SLAVE_Z_ID       = 0x73,
    }SLAVE_ID;

    typedef enum{
        PWM_X            = 0,
        PWM_Y            = 1,
        PWM_Z            = 2,
        EN_X             = 3,
        EN_Y             = 4,
        EN_Z             = 5,
    }PIN;

    /*Register*/
    typedef enum{
        REG_Status                              = 0x00,
        REG_Mode                                = 0x01,
        REG_RealTimePlaybackInput               = 0x02,
        REG_LibrarySelection                    = 0x03,
        REG_WaveformSequencer_Head              = 0x04,             // start from 0x04 to 0x0b , total eight in sequence
        REG_Go                                  = 0x0C,

        //Time offsets
        REG_OverdriveTimeOffset                 = 0x0D,
        REG_SustainTimeOffsetPositive           = 0x0E,
        REG_SustainTimeOffsetNegative           = 0x0F,
        REG_BrakeTimeOffset                     = 0x10,

        //Audio related(useless?)
        REG_Audio2VibeControl                   = 0x11,
        REG_Audio2VibeMinimumInputLevel         = 0x12,
        REG_Audio2VibeMaximumIuputLevel         = 0x13,
        REG_Audio2VibeMinimumOuputDrive         = 0x14,
        REG_Audio2VibeMaximumOuputDrive         = 0x15,

        //Auto calibration or control related(important)
        REG_RatedVoltage                        = 0x16,
        REG_OverdriveClampVoltage               = 0x17,
        REG_AutoCalibrationCompensationResult   = 0x18,
        REG_AutoCalibrationBackEMFResult        = 0x19,
        REG_FeedbackControl                     = 0x1A,
        REG_Control1                            = 0x1B,
        REG_Control2                            = 0x1C,
        REG_Control3                            = 0x1D,
        REG_Control4                            = 0x1E,
        REG_Control5                            = 0x1F,

        //LRA related
        REG_LRAOpenLoopPeriod                   = 0x20,
        REG_VbatVoltageMonitor                  = 0x21,
        REG_LRAResonancePeriod                  = 0x22,
        REG_MAX                                 = REG_LRAResonancePeriod,
        REG_NUM                                 = REG_MAX+1,

        //Register in word
        //Mode 
        MODE_DEV_RESET_reset                    = 0x80,
        MODE_DEV_RESET_default                  = 0x00,
        /*----------------------------------------------------------*/
        MODE_STANDBY_standby                    = 0x40,
        MODE_STANDBY_ready                      = 0x00,
        MODE_STANDBY_default                    = MODE_STANDBY_standby,
        /*----------------------------------------------------------*/
        MODE_MODE_internal_trigger              = 0x00,
        MODE_MODE_external_trigger_edge         = 0x01,
        MODE_MODE_external_trigger_level        = 0x02,  
        MODE_MODE_pwm_or_analog                 = 0x03,  /*need to set C3*/
        MODE_MODE_audio2vibe                    = 0x04,  /*need to set C3*/
        MODE_MODE_realtime_playback             = 0x05,
        MODE_MODE_diagnostics                   = 0x06,
        MODE_MODE_auto_calibration              = 0x07,  /*need to set FC*/
        MODE_MODE_default                       = MODE_MODE_internal_trigger,
        
        //Real-Time Playback Input
        RTP_RTP_INPUT_default                   = 0x00, 

        //Library Selection 
        LS_HI_Z_hi_z                            = 0x10,
        LS_HI_Z_low_z                           = 0x00,
        LS_HI_Z_default                         = LS_HI_Z_low_z,
        /*----------------------------------------------------------*/
        LS_LIBRARY_SEL_empty                    = 0x00,
        LS_LIBRARY_SEL_a                        = 0x01,
        LS_LIBRARY_SEL_b                        = 0x02,
        LS_LIBRARY_SEL_c                        = 0x03,
        LS_LIBRARY_SEL_d                        = 0x04,
        LS_LIBRARY_SEL_e                        = 0x05,
        LS_LIBRARY_SEL_f                        = 0x07,
        LS_LIBRARY_SEL_lra                      = 0x06,
        LS_LIBRARY_SEL_default                  = LS_LIBRARY_SEL_a,

        //Waveform Sequencer 0x04 - 0x0B
        WS_WAIT_wait                            = 0x80, /*10ms*[0:6]*/
        WS_WAIT_do                              = 0x00,
        WS_WAIT_default                         = WS_WAIT_do,
        WS_NUM                                  = 8,
        /*----------------------------------------------------------*/
        WS_WAV_FRM_SEQ_default                  = 0x00,

        //Go, need to set this if you use internal trigger, diagnostics or auto calibration
        GO_GO_go                                = 0x01,
        GO_GO_stop                              = 0x00,
        GO_GO_default                           = GO_GO_stop,

        //Overdrive Time Offset, need to set C5 first, ref to page.38
        ODT_ODT_default                         = 0x00,

        //Sustain Time Offset Positive, need to set C5 first
        SPT_SPT_default                         = 0x00,

        //Sustain Time Offset Negative, need to set C5 first
        SNT_SNT_default                         = 0x00,

        //Brake Time Offset, need to set C5 first
        BRT_BRT_default                         = 0x00,

        //Audio-to-Vibe Control
        A2VC_ATH_PEAK_TIME_10ms                 = 0x00,
        A2VC_ATH_PEAK_TIME_20ms                 = 0x04,
        A2VC_ATH_PEAK_TIME_30ms                 = 0x08,
        A2VC_ATH_PEAK_TIME_40ms                 = 0x0c,
        A2VC_ATH_PEAK_TIME_default              = A2VC_ATH_PEAK_TIME_20ms,
        /*----------------------------------------------------------*/
        A2VC_ATH_FILTER_100hz                   = 0x00,
        A2VC_ATH_FILTER_125hz                   = 0x01,
        A2VC_ATH_FILTER_150hz                   = 0x02,
        A2VC_ATH_FILTER_200hz                   = 0x03,
        A2VC_ATH_FILTER_default                 = A2VC_ATH_FILTER_125hz,

        //ATH_MIN_INPUT
        A2VMinIL_ATH_MIN_INPUT_default          = 0x19,

        //ATH_MAX_INPUT
        A2VMaxIL_ATH_MAX_INPUT_default          = 0xff,

        //ATH_MIN_DRIVE
        A2VMinOD_ATH_MIN_DRIVE_default          = 0x19,

        //ATH_MAX_DRIVE
        A2VMaxOD_ATH_MAX_DRIVE_default          = 0xff,

        //Rated Voltage, need to see page.22 formula
        RV_RATED_VOLTAGE_default                = 0x3e,

        //Overdrive Voltage-Clamped, need to see page.23 formula
        ODC_OD_CLAMP_default                    = 0x8c,

        //Auto-Calibration Compensation Result, should be valid & get only after auto calibration?
        ACCR_A_CAL_COMP_default                 = 0x0c,          

        //Auto-Calibration Back-EMF Result, should be valid & get only after auto calibration?
        ACBR_A_CAL_BEM_default                  = 0x6f,

        //FeedBack Control
        FC_N_ERM_LRA_erm                         = 0x00,
        FC_N_ERM_LRA_lra                         = 0x80,
        /*----------------------------------------------------------*/
        FC_FB_BRAKE_FACTOR_1x                    = 0x00,
        FC_FB_BRAKE_FACTOR_2x                    = 0b00010000,
        FC_FB_BRAKE_FACTOR_3x                    = 0b00100000,
        FC_FB_BRAKE_FACTOR_4x                    = 0b00110000,
        FC_FB_BRAKE_FACTOR_6x                    = 0b01000000,
        FC_FB_BRAKE_FACTOR_8x                    = 0b01010000,
        FC_FB_BRAKE_FACTOR_16x                   = 0b01100000,
        FC_FB_BRAKE_FACTOR_braking_disabled      = 0b01110000,
        FC_FB_BRAKE_FACTOR_default               = FC_FB_BRAKE_FACTOR_4x,
        /*----------------------------------------------------------*/
        FC_LOOP_GAIN_low                         = 0x00,
        FC_LOOP_GAIN_medium                      = 0b00000100,
        FC_LOOP_GAIN_high                        = 0b00001000,
        FC_LOOP_GAIN_very_high                   = 0b00001100,
        FC_LOOP_GAIN_default                     = FC_LOOP_GAIN_medium,
        /*----------------------------------------------------------*/
        FC_BEMF_GAIN_low                         = 0x00,
        FC_BEMF_GAIN_medium                      = 0x01,
        FC_BEMF_GAIN_high                        = 0x02,
        FC_BEMF_GAIN_very_high                   = 0x03,    //look at page.43
        FC_BEMF_GAIN_default                     = FC_BEMF_GAIN_high,

        //Control 1
        C1_STARTUP_BOOST_on                      = 0x80,
        C1_STARTUP_BOOST_off                     = 0x00,
        /*----------------------------------------------------------*/
        C1_AC_COUPLE_on                          = 0x40,
        C1_AC_COUPLE_off                         = 0x00,    //on only if using 'analog' input signal
        /*----------------------------------------------------------*/
        C1_DRIVE_TIME_default                    = 0x13,
        
        //Control 2
        C2_BIDIR_INPUT_on                        = 0x80,    //both
        C2_BIDIR_INPUT_off                       = 0x00,    //only for close loop
        C2_BIDIR_INPUT_default                   = C2_BIDIR_INPUT_on,
        /*----------------------------------------------------------*/
        C2_BRAKE_STABILIZER_on                   = 0x40,    //braking gain down when almost finish
        C2_BRAKE_STABILIZER_off                  = 0x00,
        C2_BRAKE_STABILIZER_default              = C2_BRAKE_STABILIZER_on,
        /*----------------------------------------------------------*/
        C2_SAMPLE_TIME_150us                     = 0x00,
        C2_SAMPLE_TIME_200us                     = 0b00010000,
        C2_SAMPLE_TIME_250us                     = 0b00100000,
        C2_SAMPLE_TIME_300us                     = 0b00110000,
        C2_SAMPLE_TIME_default                   = C2_SAMPLE_TIME_200us,
        /*----------------------------------------------------------*/
        C2_BLANKING_TIME_erm_45us                = 0x00,    
        C2_BLANKING_TIME_erm_75us                = 0x01,
        C2_BLANKING_TIME_erm_150us               = 0x02,
        C2_BLANKING_TIME_erm_225us               = 0x03,
        C2_BLANKING_TIME_erm_default             = C2_BLANKING_TIME_erm_150us,
        C2_BLANKING_TIME_lra_15us                = 0x00,    /*need to set C5 too, if in LRA mode*/
        C2_BLANKING_TIME_lra_25us                = 0x01,
        C2_BLANKING_TIME_lra_50us                = 0x02,
        C2_BLANKING_TIME_lra_75us                = 0x03,
        C2_BLANKING_TIME_lra_90us                = 0x00,
        C2_BLANKING_TIME_lra_105us               = 0x01,
        C2_BLANKING_TIME_lra_120us               = 0x02,
        C2_BLANKING_TIME_lra_135us               = 0x03,
        C2_BLANKING_TIME_lra_150us               = 0x00,
        C2_BLANKING_TIME_lra_165us               = 0x01,
        C2_BLANKING_TIME_lra_180us               = 0x02,
        C2_BLANKING_TIME_lra_195us               = 0x03,
        C2_BLANKING_TIME_lra_210us               = 0x00,
        C2_BLANKING_TIME_lra_235us               = 0x01,
        C2_BLANKING_TIME_lra_260us               = 0x02,
        C2_BLANKING_TIME_lra_285us               = 0x03,
        C2_BLANKING_TIME_lra_default            = C2_BLANKING_TIME_lra_50us,

        /*----------------------------------------------------------*/
        C2_IDISS_TIME_erm_45us                   = 0x00, 
        C2_IDISS_TIME_erm_75us                   = 0x01,
        C2_IDISS_TIME_erm_150us                  = 0x02,
        C2_IDISS_TIME_erm_225us                  = 0x03,
        C2_IDISS_TIME_erm_default                = C2_IDISS_TIME_erm_150us,
        C2_IDISS_TIME_lra_15us                   = 0x00,    /*need to set C5 too, if in LRA mode*/
        C2_IDISS_TIME_lra_25us                   = 0x01,
        C2_IDISS_TIME_lra_50us                   = 0x02,
        C2_IDISS_TIME_lra_75us                   = 0x03,
        C2_IDISS_TIME_lra_90us                   = 0x00,
        C2_IDISS_TIME_lra_105us                  = 0x01,
        C2_IDISS_TIME_lra_120us                  = 0x02,
        C2_IDISS_TIME_lra_135us                  = 0x03,
        C2_IDISS_TIME_lra_150us                  = 0x00,
        C2_IDISS_TIME_lra_165us                  = 0x01,
        C2_IDISS_TIME_lra_180us                  = 0x02,
        C2_IDISS_TIME_lra_195us                  = 0x03,
        C2_IDISS_TIME_lra_210us                  = 0x00,
        C2_IDISS_TIME_lra_235us                  = 0x01,
        C2_IDISS_TIME_lra_260us                  = 0x02,
        C2_IDISS_TIME_lra_280us                  = 0x03,
        C2_IDISS_TIME_lra_default                = C2_IDISS_TIME_lra_50us,

        //Control 3
        C3_NG_THRESH_disabled                    = 0x00,
        C3_NG_THRESH_2pct                        = 0x40,     /*2%*/
        C3_NG_THRESH_4pct                        = 0x80,
        C3_NG_THRESH_8pct                        = 0b11000000,     
        C3_NG_THRESH_default                     = C3_NG_THRESH_4pct,
        /*----------------------------------------------------------*/
        C3_ERM_OPEN_LOOP_open                    = 0b00100000,
        C3_ERM_OPEN_LOOP_close                   = 0x00,
        C3_ERM_OPEN_LOOP_default                 = C3_ERM_OPEN_LOOP_open,
        /*----------------------------------------------------------*/
        C3_SUPPLY_COMP_DIS_enabled               = 0x00,
        C3_SUPPLY_COMP_DIS_disabled              = 0b00010000,
        C3_SUPPLY_COMP_DIS_default               = C3_SUPPLY_COMP_DIS_enabled,
        /*----------------------------------------------------------*/
        C3_DATA_FORMAT_RTP_signed                = 0x00,
        C3_DATA_FORMAT_RTP_unsigned              = 0b00001000,  /*RTP data unsigned or signed*/
        C3_DATA_FORMAT_RTP_default               = C3_DATA_FORMAT_RTP_signed,
        /*----------------------------------------------------------*/
        C3_LRA_DRIVE_MODE_once_per_cycle         = 0x00,
        C3_LRA_DRIVE_MODE_twice_per_cycle        = 0b00000100,
        C3_LRA_DRIVE_MODE_default                = C3_LRA_DRIVE_MODE_once_per_cycle,
        /*----------------------------------------------------------*/
        C3_N_PWM_ANALOG_pwm                      = 0x00,
        C3_N_PWM_ANALOG_analog                   = 0x02,
        C3_N_PWM_ANALOG_default                  = 0x00,               
        /*----------------------------------------------------------*/
        C3_LRA_OPEN_LOOP_auto                    = 0x00,
        C3_LRA_OPEN_LOOP_open                    = 0x01,
        C3_LRA_OPEN_LOOP_default                 = C3_LRA_OPEN_LOOP_auto,

        //Control 4
        C4_ZC_DET_TIME_100us                     = 0x00,
        C4_ZC_DET_TIME_200us                     = 0b01000000,
        C4_ZC_DET_TIME_300us                     = 0b10000000,
        C4_ZC_DET_TIME_390us                     = 0b11000000,
        C4_ZC_DET_TIME_default                   = C4_ZC_DET_TIME_100us,
        /*----------------------------------------------------------*/
        C4_AUTO_CAL_TIME_150To350ms              = 0x00,
        C4_AUTO_CAL_TIME_250To450ms              = 0b00010000,
        C4_AUTO_CAL_TIME_500To700ms              = 0b00100000,
        C4_AUTO_CAL_TIME_1000To1200ms            = 0b00110000,
        C4_AUTO_CAL_TIME_default                 = C4_AUTO_CAL_TIME_500To700ms,
        /*----------------------------------------------------------*/
        C4_OTP_PROGRAM_default                   = 0x00,

        //Control 5
        C5_AUTO_OL_CNT_3times                    = 0x00,
        C5_AUTO_OL_CNT_4times                    = 0x40,
        C5_AUTO_OL_CNT_5times                    = 0x80,
        C5_AUTO_OL_CNT_6times                    = 0b11000000,
        C5_AUTO_OL_CNT_default                   = C5_AUTO_OL_CNT_5times,
        /*----------------------------------------------------------*/
        C5_LRA_AUTO_OPEN_LOOP_never              = 0x00,                 
        C5_LRA_AUTO_OPEN_LOOP_auto               = 0b00100000,
        C5_LRA_AUTO_OPEN_LOOP_default            = C5_LRA_AUTO_OPEN_LOOP_never,/*LRA only*/
        /*----------------------------------------------------------*/
        C5_PLAYBACK_INTERVAL_5ms                 = 0x00,
        C5_PLAYBACK_INTERVAL_1ms                 = 0x10,
        C5_PLAYBACK_INTERVAL_default             = C5_PLAYBACK_INTERVAL_5ms,
        /*----------------------------------------------------------*/
        C5_BLANKING_TIME_lra_15us                = 0x00,    /*need to set C2 too, if in LRA mode*/
        C5_BLANKING_TIME_lra_25us                = 0x00,
        C5_BLANKING_TIME_lra_50us                = 0x00,
        C5_BLANKING_TIME_lra_75us                = 0x00,
        C5_BLANKING_TIME_lra_90us                = 0b00000100,
        C5_BLANKING_TIME_lra_105us               = 0b00000100,
        C5_BLANKING_TIME_lra_120us               = 0b00000100,
        C5_BLANKING_TIME_lra_135us               = 0b00000100,
        C5_BLANKING_TIME_lra_150us               = 0b00001000,
        C5_BLANKING_TIME_lra_165us               = 0b00001000,
        C5_BLANKING_TIME_lra_180us               = 0b00001000,
        C5_BLANKING_TIME_lra_195us               = 0b00001000,
        C5_BLANKING_TIME_lra_210us               = 0b00001100,
        C5_BLANKING_TIME_lra_235us               = 0b00001100,
        C5_BLANKING_TIME_lra_260us               = 0b00001100,
        C5_BLANKING_TIME_lra_285us               = 0b00001100,
        C5_BLANKING_TIME_lra_default             = C5_BLANKING_TIME_lra_50us,
        /*----------------------------------------------------------*/
        C5_IDISS_TIME_lra_15us                   = 0x00,    /*need to set C5 too, if in LRA mode*/
        C5_IDISS_TIME_lra_25us                   = 0x00,
        C5_IDISS_TIME_lra_50us                   = 0x00,
        C5_IDISS_TIME_lra_75us                   = 0x00,
        C5_IDISS_TIME_lra_90us                   = 0x01,
        C5_IDISS_TIME_lra_105us                  = 0x01,
        C5_IDISS_TIME_lra_120us                  = 0x01,
        C5_IDISS_TIME_lra_135us                  = 0x01,
        C5_IDISS_TIME_lra_150us                  = 0x02,
        C5_IDISS_TIME_lra_165us                  = 0x02,
        C5_IDISS_TIME_lra_180us                  = 0x02,
        C5_IDISS_TIME_lra_195us                  = 0x02,
        C5_IDISS_TIME_lra_210us                  = 0x03,
        C5_IDISS_TIME_lra_235us                  = 0x03,
        C5_IDISS_TIME_lra_260us                  = 0x03,
        C5_IDISS_TIME_lra_280us                  = 0x03,
        C5_IDISS_TIME_lra_default                = C5_IDISS_TIME_lra_50us,

        //LRA Open Loop Period
        LRAOLP_OL_LRA_PERIOD_default             = 0x00,

        //Vbat Voltage Monitor
        VVM_VBAT_default                         = 0x00,

        //LRA Resonance Period                  
        LRARP_LRA_PERIOD_default                 = 0x00,    /*only valid when actively sending a waveform*/
    }REGISTER;

    uint8_t Default_Value[REG_NUM] = {
        0xe0, 0x40, 0x0,  0x1,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 
        0x0,  0x05, 0x19, 0xff, 0x19, 0xff, 0x3e, 0x8c, 0x0c, 0x6c, 0x36, 0x93, 0xf5, 0xa0, 0x20, 0x80,
        0x0,  0x0 , 0x0
    };

    /*class*/
    class DRV2605L{
        public:
            DRV2605L();
            DRV2605L(int EN_pin,int slave_id = SLAVE_DEFAULT_ID);
            ~DRV2605L();

            bool is_init = 0;
            int slave_id;
            int EN_pin;
            uint16_t errCode;

            /**
             * @brief 
             * 
             */
            void init();

            /**
             * @brief 
             * 
             * @param reg_addr 
             * @param buf 
             * @param len 
             * @return ssize_t 
             */
            ssize_t read(uint32_t reg_addr,void *buf,size_t len);

             /**
             * @brief read function based on i2c library(single addr start, mutiple buf read)
             * 
             * @param reg_addr 
             * @return ssize_t 
             */
            uint8_t read(uint32_t reg_addr);

            /**
             * @brief 
             * 
             */
            void print_all_register();

            /**
             * @brief 
             * 
             * @param reg_addr 
             * @return ssize_t 
             */
            ssize_t write(uint32_t reg_addr, const void* content,size_t len);

            /**
             * @brief 
             * 
             * @param reg_addr 
             * @param content 
             * @return ssize_t 
             */
            ssize_t write(uint32_t reg_addr, uint8_t content);

            /**
             * @brief  Set DRV main properties of taptic engine 6s(LRA)
             * 
             * @return ssize_t 
             */
            void set_LRA_6s();        

            /*Setting function*/
            void hard_reset();       /*Write 0x01 with 0x80*/
            void run();               /*Set go bit*/
            void run_autoCalibration();  /*Set auto calibration related registers*/
            void run_RTPtest();     

            void stop();             /*Cancel go bit*/

            void set_amplitude(uint8_t);

            /*Get function*/
            uint8_t get_ACCR();          /*Get Auto-Calibration Compensation Result*/
            uint8_t get_ACBR();          /*Get Auto-Calibration Back-EMF Result*/
            uint8_t get_VVM();           /*Get Vbat Voltage Monitor*/
            double  get_operating_hz();         /*Get Period of LRA after auto calibration*/
            void get_auto_calibration_info();

        protected:
            I2CDevice i2c;
            
            /*Register related*/
            void set_MODE(uint8_t);      /*Mode*/
            void set_RTP(uint8_t);       /*Real-Time Playback Input*/
            void set_LS(uint8_t);        /*Library Selection*/
            void set_WS(uint8_t,uint8_t);/*Waveform Sequencer*/
            void set_GO(uint8_t);         /*Go*/
            void set_ODT(uint8_t);       /*Overdrive Time Offset*/
            void set_SPT(uint8_t);       /*Sustain Time Positive Offset*/
            void set_SNT(uint8_t);       /*Sustain Time Negative Offset*/
            void set_BRT(uint8_t);       /*Brake Time*/
            void set_A2VC(uint8_t);      /*Audio-to-Vibe Control*/
            void set_A2VMinIL(uint8_t);  /*Audio-to-Vibe Minimum Input Level*/
            void set_A2VMaxIL(uint8_t);  /*Audio-to-Vibe Maximum Input Level*/ 
            void set_A2VMinOD(uint8_t);  /*Audio-to-Vibe Minimum Output Drive*/
            void set_A2VMaxOD(uint8_t);  /*Audio-to-Vibe Maximum Output Drive*/ 
            void set_RV(uint8_t);        /*Rated Voltage -> use in auto calibration*/
            void set_ODC(uint8_t);       /*Overdrive Clamp Voltage*/
            void set_FC(uint8_t);        /*Feedback Control*/
            void set_C1(uint8_t);        /*Control 1*/
            void set_C2(uint8_t);        /*Control 2*/
            void set_C3(uint8_t);        /*Control 3*/
            void set_C4(uint8_t);        /*Control 4*/
            void set_C5(uint8_t);        /*Control 5*/
            void set_LRAOLP(uint8_t);    /*LRA open loop period*/
            void set_LRARP(uint8_t);     /*LRA resonance period*/
            void set_ACCR(uint8_t);      /*Auto-Calibration Compensation Result*/
            void set_ACBR(uint8_t);      /*Auto-Calibration Back-EMF Result*/ 
            void set_VVM(uint8_t);       /*Vbat Voltage Monitor*/

        private:
            void info(uint32_t reg_addr,uint8_t content);
            void set(uint32_t reg_addr,uint8_t);
    };
}
#endif