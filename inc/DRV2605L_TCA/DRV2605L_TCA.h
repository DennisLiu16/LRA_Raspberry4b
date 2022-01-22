/**
 * @file DRV2605L_TCA.h
 * @author Dennis Liu (liusx880630@gmail.com)
 * @brief For mutiple DRV2605L with TCA9548A I2C multiplexer
 * @version 0.1
 * @date 2022-01-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _LRA_DRV2605L_TCA_H_
#define _LRA_DRV2605L_TCA_H_

#include <I2c/i2c.h>
#include <ErrorCode/LRA_ErrorCode.h>
#include <PI/LRA_PI_Util.h>
extern "C" {
#include <wiringPi.h>
#include <signal.h>
}

namespace LRA_DRV2605L_TCA
{
    using namespace LRA_PI_Util;
    using namespace LRA_Error;

    class DRV2605L_TCA
    {
        public:
        ////////////////
        /*Const region*/
        ////////////////

        typedef enum{
            // I2C slave id (slave device address)
            TCA_SLAVE_ID = 0x70,
            DRV_SLAVE_ID = 0x5a,

            // TCA9548A channel
            LRA_X_ch = 0,
            LRA_Y_ch = 1,
            LRA_Z_ch = 2,

            // EN
            EN_X = 0,
            EN_Y = 1,
            EN_Z = 2,
        }Default;

        typedef enum{
            DEVICE_ID = 0, 
            DIAG_RESULT = 1,
            OVER_TEMP = 2,
            OC_DETECT = 3,
            DEV_RESET = 4,
            STANDBY = 5,
            MODE = 6,
            RTP_INPUT = 7,
            HI_Z = 8,
            LIBRARY_SEL2 = 9,
            LIBRARY_SEL1 = 10,
            LIBRARY_SEL0 = 11,
            WAIT1 = 12,
            WAV_FRM_SEQ1 = 13,
            WAIT2 = 14,
            WAV_FRM_SEQ2 = 15,
            WAIT3 = 16,
            WAV_FRM_SEQ3 = 17,
            WAIT4 = 18,
            WAV_FRM_SEQ4 = 19,
            WAIT5 = 20,
            WAV_FRM_SEQ5 = 21,
            WAIT6 = 22,
            WAV_FRM_SEQ6 = 23,
            WAIT7 = 24,
            WAV_FRM_SEQ7 = 25,
            WAIT8 = 26,
            WAV_FRM_SEQ8 = 27,
            GO = 28,
            ODT = 29,
            SPT = 30,
            SNT = 31,
            BRT = 32,
            ATH_PEAK_TIME = 33,
            ATH_FILTER = 34,
            ATH_MIN_INPUT = 35,
            ATH_MAX_INPUT = 36,
            ATH_MIN_DRIVE = 37,
            ATH_MAX_DRIVE = 38,
            RATED_VOLTAGE = 39,
            OD_CLAMP = 40,
            A_CAL_COMP = 41,
            A_CAL_BEMF = 42,
            N_ERM_LRA = 43,
            FB_BRAKE_FACTOR = 44,
            LOOP_GAIN = 45,
            BEMF_GAIN = 46,
            STARTUP_BOOST = 47,
            AC_COUPLE = 48,
            DRIVE_TIME = 49,        // best ~ 0.5*LRA_PERIOD, p44
            BIDIR_INPUT = 50,
            BRAKE_STABILIZER = 51,
            SAMPLE_TIME = 52,
            BLANKING_TIME0 = 53,
            IDISS_TIME0 = 54,
            NG_THRESH = 55,
            ERM_OPEN_LOOP = 56,
            SUPPLY_COMP_DIS = 57,
            DATA_FORMAT_RTP = 58,
            LRA_DRIVE_MODE = 59,
            N_PWM_ANALOG = 60,
            LRA_OPEN_LOOP = 61,
            ZC_DET_TIME = 62,
            AUTO_CAL_TIME = 63,
            OTP_STATUS = 64,
            OTP_PROGRAM = 65,
            AUTO_OL_CNT = 66,
            LRA_AUTO_OPEN_LOOP = 67,
            PLAYBACK_INTERVAL = 68,
            BLANKING_TIME1 = 69,
            IDISS_TIME1 = 70,
            OL_LRA_PERIOD = 71,
            VBAT = 72,
            LRA_PERIOD = 73,

            /*You can extend your register here*/

            NUM_REG
        }regIndex;

        typedef enum{
            Status                              = 0x00,
            Mode                                = 0x01,
            RealTimePlaybackInput               = 0x02,
            LibrarySelection                    = 0x03,
            WaveformSequencer_Head              = 0x04,             // start from 0x04 to 0x0b , total eight in sequence
            Go                                  = 0x0C,

            //Time offsets
            OverdriveTimeOffset                 = 0x0D,
            SustainTimeOffsetPositive           = 0x0E,
            SustainTimeOffsetNegative           = 0x0F,
            BrakeTimeOffset                     = 0x10,

            //Audio related(useless?)
            Audio2VibeControl                   = 0x11,
            Audio2VibeMinimumInputLevel         = 0x12,
            Audio2VibeMaximumIuputLevel         = 0x13,
            Audio2VibeMinimumOuputDrive         = 0x14,
            Audio2VibeMaximumOuputDrive         = 0x15,

            //Auto calibration or control related(important)
            RatedVoltage                        = 0x16,
            OverdriveClampVoltage               = 0x17,
            AutoCalibrationCompensationResult   = 0x18,
            AutoCalibrationBackEMFResult        = 0x19,
            FeedbackControl                     = 0x1A,
            Control1                            = 0x1B,
            Control2                            = 0x1C,
            Control3                            = 0x1D,
            Control4                            = 0x1E,
            Control5                            = 0x1F,

            //LRA related
            LRAOpenLoopPeriod                   = 0x20,
            VbatVoltageMonitor                  = 0x21,
            LRAResonancePeriod                  = 0x22,
        }Addr;

        typedef enum{
            /*Mode*/
            DEV_RESET_reset                     = 0x01,
            DEV_RESET_empty                     = 0x00,
            STANDBY_standby                     = 0x01,
            STANDBY_ready                       = 0x00,
            MODE_internalTrigger                = 0x00,
            MODE_external_trigger_edge          = 0x01,
            MODE_external_trigger_level         = 0x02,  
            MODE_pwm_or_analog                  = 0x03,  /*need to set Control3*/
            MODE_audio2vibe                     = 0x04,  /*need to set Control3*/
            MODE_realtime_playback              = 0x05,
            MODE_diagnostics                    = 0x06,
            MODE_auto_calibration               = 0x07,  /*need to set FeedbackControl*/

            /*Library Selection*/
            HI_Z_high                           = 0x01,
            HI_Z_low                            = 0x00,
            LIBRARY_SEL_empty                   = 0x00,
            LIBRARY_SEL_a                       = 0x01,
            LIBRARY_SEL_b                       = 0x02,
            LIBRARY_SEL_c                       = 0x03,
            LIBRARY_SEL_d                       = 0x04,
            LIBRARY_SEL_e                       = 0x05,
            LIBRARY_SEL_f                       = 0x07,
            LIBRARY_SEL_lra                     = 0x06,

            /*Waveform Sequencer 0x04 - 0x0B*/
            WAIT_wait                           = 0x01,
            WAIT_empty                          = 0x00,

            /*Go*/
            //need to set this if you use internal trigger, diagnostics or auto calibration
            GO_go                               = 0x01,
            GO_stop                             = 0x00,

            /*Audio-to-Vibe Control*/
            ATH_PEAK_TIME_10ms                  = 0x00,
            ATH_PEAK_TIME_20ms                  = 0x01,
            ATH_PEAK_TIME_30ms                  = 0x02,
            ATH_PEAK_TIME_40ms                  = 0x03,
            A2VC_ATH_FILTER_100hz               = 0x00,
            A2VC_ATH_FILTER_125hz               = 0x01,
            A2VC_ATH_FILTER_150hz               = 0x02,
            A2VC_ATH_FILTER_200hz               = 0x03,

            /*FeedbackControl(FC)*/
            N_ERM_LRA_erm                       = 0x00,
            N_ERM_LRA_lra                       = 0x01,
            FB_BRAKE_FACTOR_1x                  = 0x00,
            FB_BRAKE_FACTOR_2x                  = 0x01,
            FB_BRAKE_FACTOR_3x                  = 0x02,
            FB_BRAKE_FACTOR_4x                  = 0x03,
            FB_BRAKE_FACTOR_6x                  = 0x04,
            FB_BRAKE_FACTOR_8x                  = 0x05,
            FB_BRAKE_FACTOR_16x                 = 0x06,
            FB_BRAKE_FACTOR_disabled            = 0x07,
            LOOP_GAIN_low                       = 0x00,
            LOOP_GAIN_medium                    = 0x01,
            LOOP_GAIN_high                      = 0x02,
            LOOP_GAIN_veryhi                    = 0x03,
            BEMF_GAIN_low                       = 0x00,
            BEMF_GAIN_medium                    = 0x01,
            BEMF_GAIN_high                      = 0x02,
            BEMF_GAIN_veryhi                    = 0x03, //look at page.43

            /*Control1(C1)*/
            STARTUP_BOOST_on                    = 0x01,
            STARTUP_BOOST_off                   = 0x00,
            AC_COUPLE_on                        = 0x01, //on only if using 'analog' input signal
            AC_COUPLE_off                       = 0x00, 

            /*Control2(C2)*/
            BIDIR_INPUT_on                      = 0x01,
            BIDIR_INPUT_off                     = 0x00, //only for close loop
            BRAKE_STABILIZER_on                 = 0x01, //braking gain down when almost finish
            BRAKE_STABILIZER_off                = 0x00,
            SAMPLE_TIME_150us                   = 0x00,
            SAMPLE_TIME_200us                   = 0x01,
            SAMPLE_TIME_250us                   = 0x02,
            SAMPLE_TIME_300us                   = 0x03,
            BLANKING_TIME0_erm_45us             = 0x00,
            BLANKING_TIME0_erm_75us             = 0x01,
            BLANKING_TIME0_erm_150us            = 0x02,
            BLANKING_TIME0_erm_225us            = 0x03,
            BLANKING_TIME0_lra_15us             = 0x00,
            BLANKING_TIME0_lra_25us             = 0x01,
            BLANKING_TIME0_lra_50us             = 0x02,
            BLANKING_TIME0_lra_75us             = 0x03,
            BLANKING_TIME0_lra_90us             = 0x00,
            BLANKING_TIME0_lra_105us            = 0x01,
            BLANKING_TIME0_lra_120us            = 0x02,
            BLANKING_TIME0_lra_135us            = 0x03,
            BLANKING_TIME0_lra_150us            = 0x00,
            BLANKING_TIME0_lra_165us            = 0x01,
            BLANKING_TIME0_lra_180us            = 0x02,
            BLANKING_TIME0_lra_195us            = 0x03,
            BLANKING_TIME0_lra_210us            = 0x00,
            BLANKING_TIME0_lra_235us            = 0x01,
            BLANKING_TIME0_lra_260us            = 0x02,
            BLANKING_TIME0_lra_285us            = 0x03,
            IDISS_TIME0_erm_45us                = 0x00,
            IDISS_TIME0_erm_75us                = 0x01,
            IDISS_TIME0_erm_150us               = 0x02,
            IDISS_TIME0_erm_225us               = 0x03,
            IDISS_TIME0_lra_15us                = 0x00,
            IDISS_TIME0_lra_25us                = 0x01,
            IDISS_TIME0_lra_50us                = 0x02,
            IDISS_TIME0_lra_75us                = 0x03,
            IDISS_TIME0_lra_90us                = 0x00,
            IDISS_TIME0_lra_105us               = 0x01,
            IDISS_TIME0_lra_120us               = 0x02,
            IDISS_TIME0_lra_135us               = 0x03,
            IDISS_TIME0_lra_150us               = 0x00,
            IDISS_TIME0_lra_165us               = 0x01,
            IDISS_TIME0_lra_180us               = 0x02,
            IDISS_TIME0_lra_195us               = 0x03,
            IDISS_TIME0_lra_210us               = 0x00,
            IDISS_TIME0_lra_235us               = 0x01,
            IDISS_TIME0_lra_260us               = 0x02,
            IDISS_TIME0_lra_285us               = 0x03,

            /*Control3(C3)*/
            NG_THRESH_disabled                  = 0x00,
            NG_THRESH_2pct                      = 0x01, // 2%
            NG_THRESH_4pct                      = 0x02,
            NG_THRESH_8pct                      = 0x03,
            ERM_OPEN_LOOP_open                  = 0x01,
            ERM_OPEN_LOOP_close                 = 0x00,
            SUPPLY_COMP_DIS_enabled             = 0x00, //supply compensation
            SUPPLY_COMP_DIS_disabled            = 0x01, 
            DATA_FORMAT_RTP_signed              = 0x00, //RTP data unsigned or signed
            DATA_FORMAT_RTP_unsigned            = 0x01,  
            LRA_DRIVE_MODE_1pc                  = 0x00, //once per cycle
            LRA_DRIVE_MODE_2pc                  = 0x01, //twice per cycle
            N_PWM_ANALOG_pwm                    = 0x00,
            N_PWM_ANALOG_analog                 = 0x01,
            LRA_OPEN_LOOP_auto                  = 0x00,
            LRA_OPEN_LOOP_open                  = 0x01,

            /*Control4(C4)*/
            ZC_DET_TIME_100us                   = 0x00, //zero crossing min length of time
            ZC_DET_TIME_200us                   = 0x01,
            ZC_DET_TIME_300us                   = 0x02,
            ZC_DET_TIME_390us                   = 0x03,
            AUTO_CAL_TIME_150To350ms            = 0x00, //auto calibration time
            AUTO_CAL_TIME_250To450ms            = 0x01,
            AUTO_CAL_TIME_500To700ms            = 0x02,
            AUTO_CAL_TIME_1000To1200ms          = 0x03,
            // OTP neglect

            /*Control5(C5)*/
            AUTO_OL_CNT_3                       = 0x00,
            AUTO_OL_CNT_4                       = 0x01,
            AUTO_OL_CNT_5                       = 0x02,
            AUTO_OL_CNT_6                       = 0x03,
            LRA_AUTO_OPEN_LOOP_never            = 0x00,
            LRA_AUTO_OPEN_LOOP_auto             = 0x01,
            PLAYBACK_INTERVAL_5ms               = 0x00,
            PLAYBACK_INTERVAL_1ms               = 0x01,
            BLANKING_TIME1_erm_45us             = 0x00,
            BLANKING_TIME1_erm_75us             = 0x00,
            BLANKING_TIME1_erm_150us            = 0x00,
            BLANKING_TIME1_erm_225us            = 0x00,
            BLANKING_TIME1_lra_15us             = 0x00,
            BLANKING_TIME1_lra_25us             = 0x00,
            BLANKING_TIME1_lra_50us             = 0x00,
            BLANKING_TIME1_lra_75us             = 0x00,
            BLANKING_TIME1_lra_90us             = 0x01,
            BLANKING_TIME1_lra_105us            = 0x01,
            BLANKING_TIME1_lra_120us            = 0x01,
            BLANKING_TIME1_lra_135us            = 0x01,
            BLANKING_TIME1_lra_150us            = 0x02,
            BLANKING_TIME1_lra_165us            = 0x02,
            BLANKING_TIME1_lra_180us            = 0x02,
            BLANKING_TIME1_lra_195us            = 0x02,
            BLANKING_TIME1_lra_210us            = 0x03,
            BLANKING_TIME1_lra_235us            = 0x03,
            BLANKING_TIME1_lra_260us            = 0x03,
            BLANKING_TIME1_lra_285us            = 0x03,
            IDISS_TIME1_erm_45us                = 0x00,
            IDISS_TIME1_erm_75us                = 0x00,
            IDISS_TIME1_erm_150us               = 0x00,
            IDISS_TIME1_erm_225us               = 0x00,
            IDISS_TIME1_lra_15us                = 0x00,
            IDISS_TIME1_lra_25us                = 0x00,
            IDISS_TIME1_lra_50us                = 0x00,
            IDISS_TIME1_lra_75us                = 0x00,
            IDISS_TIME1_lra_90us                = 0x01,
            IDISS_TIME1_lra_105us               = 0x01,
            IDISS_TIME1_lra_120us               = 0x01,
            IDISS_TIME1_lra_135us               = 0x01,
            IDISS_TIME1_lra_150us               = 0x02,
            IDISS_TIME1_lra_165us               = 0x02,
            IDISS_TIME1_lra_180us               = 0x02,
            IDISS_TIME1_lra_195us               = 0x02,
            IDISS_TIME1_lra_210us               = 0x03,
            IDISS_TIME1_lra_235us               = 0x03,
            IDISS_TIME1_lra_260us               = 0x03,
            IDISS_TIME1_lra_285us               = 0x03,
        }Val;

        typedef const struct Units{
            double WAIT_ms = 10.0;                      /*ms, p37*/
            double ATH_INPUT_V = 1.8;                   /*voltage, p40*/
            double AUTO_CALIBRATION_BACK_EMF_V = 1.22;  /*voltage, p42*/
            double DRIVE_TIME_LRA_ms = 0.1;             /*ms, p44*/
            double DRIVE_TIME_ERM_ms = 0.2;             /*ms, p44*/
            double LRA_PERIOD_us = 98.46;               /*us, p50,51*/
            double VBAT_V = 5.6;                        /*V, p51*/

        }Units;

        typedef const struct Bias{
            double AUTO_CALIBRATION_COMP_COEFF = 1.0;   /*unit, p42*/
            double DRIVE_TIME_LRA_ms = 0.5;             /*ms, p44*/
            double DRIVE_TIME_ERM_ms = 1.0;             /*ms, p44*/
        }Bias;

        const uint8_t addr[NUM_REG] = {
            /*DEVICE_ID = 0, */         0x00,
            /*DIAG_RESULT = 1,*/        0x00,
            /*OVER_TEMP = 2,*/          0x00,
            /*OC_DETECT = 3,*/          0x00,
            /*DEV_RESET = 4,*/          0x01,
            /*STANDBY = 5,*/            0x01,
            /*MODE = 6,*/               0x01,
            /*RTP_INPUT = 7,*/          0x02,
            /*HI_Z = 8,*/               0x03,
            /*LIBRARY_SEL2 = 9,*/       0x03,
            /*LIBRARY_SEL1 = 10,*/      0x03,
            /*LIBRARY_SEL0 = 11,*/      0x03,
            /*WAIT1 = 12,*/             0x04,
            /*WAV_FRM_SEQ1 = 13,*/      0x04,
            /*WAIT2 = 14,*/             0x05, 
            /*WAV_FRM_SEQ2 = 15,*/      0x05,
            /*WAIT3 = 16,*/             0x06,
            /*WAV_FRM_SEQ3 = 17,*/      0x06,
            /*WAIT4 = 18,*/             0x07,
            /*WAV_FRM_SEQ4 = 19,*/      0x07,
            /*WAIT5 = 20,*/             0x08,
            /*WAV_FRM_SEQ5 = 21,*/      0x08,    
            /*WAIT6 = 22,*/             0x09,
            /*WAV_FRM_SEQ6 = 23,*/      0x09,
            /*WAIT7 = 24,*/             0x0A,
            /*WAV_FRM_SEQ7 = 25,*/      0x0A,
            /*WAIT8 = 26,*/             0x0B,
            /*WAV_FRM_SEQ8 = 27,*/      0x0B,
            /*GO = 28,*/                0x0C,
            /*ODT = 29,*/               0x0D,
            /*SPT = 30,*/               0x0E,
            /*SNT = 31,*/               0x0F,
            /*BRT = 32,*/               0x10,
            /*ATH_PEAK_TIME = 33,*/     0x11,
            /*ATH_FILTER = 34,*/        0x11,
            /*ATH_MIN_INPUT = 35,*/     0x12,
            /*ATH_MAX_INPUT = 36,*/     0x13,
            /*ATH_MIN_DRIVE = 37,*/     0x14,
            /*ATH_MAX_DRIVE = 38,*/     0x15,
            /*RATED_VOLTAGE = 39,*/     0x16,
            /*OD_CLAMP = 40,*/          0x17,        
            /*A_CAL_COMP = 41,*/        0x18,
            /*A_CAL_BEMF = 42,*/        0x19,
            /*N_ERM_LRA = 43,*/         0x1A,
            /*FB_BRAKE_FACTOR = 44,*/   0x1A,
            /*LOOP_GAIN = 45,*/         0x1A,
            /*BEMF_GAIN = 46,*/         0x1A,
            /*STARTUP_BOOST = 47,*/     0x1B,
            /*AC_COUPLE = 48,*/         0x1B,
            /*DRIVE_TIME = 49,*/        0x1B,
            /*BIDIR_INPUT = 50,*/       0x1C,
            /*BRAKE_STABILIZER = 51,*/  0x1C,
            /*SAMPLE_TIME = 52,*/       0x1C,
            /*BLANKING_TIME0 = 53,*/    0x1C,
            /*IDISS_TIME0 = 54,*/       0x1C,
            /*NG_THRESH = 55,*/         0x1D,
            /*ERM_OPEN_LOOP = 56,*/     0x1D,
            /*SUPPLY_COMP_DIS = 57,*/   0x1D,
            /*DATA_FORMAT_RTP = 58,*/   0x1D,
            /*LRA_DRIVE_MODE = 59,*/    0x1D,
            /*N_PWM_ANALOG = 60,*/      0x1D,
            /*LRA_OPEN_LOOP = 61,*/     0x1D,
            /*ZC_DET_TIME = 62,*/       0x1E,
            /*AUTO_CAL_TIME = 63,*/     0x1E,
            /*OTP_STATUS = 64,*/        0x1E,
            /*OTP_PROGRAM = 65,*/       0x1E,
            /*AUTO_OL_CNT = 66,*/       0x1F,
            /*LRA_AUTO_OPEN_LOOP = 67,*/0x1F,
            /*PLAYBACK_INTERVAL = 68,*/ 0x1F,
            /*BLANKING_TIME1 = 69,*/    0x1F,
            /*IDISS_TIME1 = 70,*/       0x1F,
            /*OL_LRA_PERIOD = 71,*/     0x20,
            /*VBAT = 72,*/              0x21,
            /*LRA_PERIOD = 73,*/        0x22,
        };

        const uint8_t startbit[NUM_REG] = {
            /*DEVICE_ID = 0, */         5,
            /*DIAG_RESULT = 1,*/        3,
            /*OVER_TEMP = 2,*/          1,
            /*OC_DETECT = 3,*/          0,
            /*DEV_RESET = 4,*/          7,
            /*STANDBY = 5,*/            6,
            /*MODE = 6,*/               0,
            /*RTP_INPUT = 7,*/          0,
            /*HI_Z = 8,*/               4,
            /*LIBRARY_SEL2 = 9,*/       2,
            /*LIBRARY_SEL1 = 10,*/      1,
            /*LIBRARY_SEL0 = 11,*/      0,
            /*WAIT1 = 12,*/             7,
            /*WAV_FRM_SEQ1 = 13,*/      0,
            /*WAIT2 = 14,*/             7,
            /*WAV_FRM_SEQ2 = 15,*/      0,
            /*WAIT3 = 16,*/             7,
            /*WAV_FRM_SEQ3 = 17,*/      0,
            /*WAIT4 = 18,*/             7,
            /*WAV_FRM_SEQ4 = 19,*/      0,
            /*WAIT5 = 20,*/             7,
            /*WAV_FRM_SEQ5 = 21,*/      0,
            /*WAIT6 = 22,*/             7,
            /*WAV_FRM_SEQ6 = 23,*/      0,
            /*WAIT7 = 24,*/             7,
            /*WAV_FRM_SEQ7 = 25,*/      0,
            /*WAIT8 = 26,*/             7,
            /*WAV_FRM_SEQ8 = 27,*/      0,
            /*GO = 28,*/                0,
            /*ODT = 29,*/               0,
            /*SPT = 30,*/               0,
            /*SNT = 31,*/               0,
            /*BRT = 32,*/               0,
            /*ATH_PEAK_TIME = 33,*/     2,
            /*ATH_FILTER = 34,*/        0,
            /*ATH_MIN_INPUT = 35,*/     0,
            /*ATH_MAX_INPUT = 36,*/     0,
            /*ATH_MIN_DRIVE = 37,*/     0,
            /*ATH_MAX_DRIVE = 38,*/     0,
            /*RATED_VOLTAGE = 39,*/     0,
            /*OD_CLAMP = 40,*/          0,
            /*A_CAL_COMP = 41,*/        0,
            /*A_CAL_BEMF = 42,*/        0,
            /*N_ERM_LRA = 43,*/         7,
            /*FB_BRAKE_FACTOR = 44,*/   4,
            /*LOOP_GAIN = 45,*/         2,
            /*BEMF_GAIN = 46,*/         0,
            /*STARTUP_BOOST = 47,*/     7,
            /*AC_COUPLE = 48,*/         5,
            /*DRIVE_TIME = 49,*/        0,
            /*BIDIR_INPUT = 50,*/       7,
            /*BRAKE_STABILIZER = 51,*/  6,
            /*SAMPLE_TIME = 52,*/       4,
            /*BLANKING_TIME0 = 53,*/    2,
            /*IDISS_TIME0 = 54,*/       0,
            /*NG_THRESH = 55,*/         6,
            /*ERM_OPEN_LOOP = 56,*/     5,
            /*SUPPLY_COMP_DIS = 57,*/   4,
            /*DATA_FORMAT_RTP = 58,*/   3,
            /*LRA_DRIVE_MODE = 59,*/    2,
            /*N_PWM_ANALOG = 60,*/      1,
            /*LRA_OPEN_LOOP = 61,*/     0,
            /*ZC_DET_TIME = 62,*/       6,
            /*AUTO_CAL_TIME = 63,*/     4,
            /*OTP_STATUS = 64,*/        2,
            /*OTP_PROGRAM = 65,*/       0,
            /*AUTO_OL_CNT = 66,*/       6,
            /*LRA_AUTO_OPEN_LOOP = 67,*/5,
            /*PLAYBACK_INTERVAL = 68,*/ 4,
            /*BLANKING_TIME1 = 69,*/    2,
            /*IDISS_TIME1 = 70,*/       0,
            /*OL_LRA_PERIOD = 71,*/     0,
            /*VBAT = 72,*/              0,
            /*LRA_PERIOD = 73,*/        0,

            /*Extend here*/
        };

        const uint8_t length[NUM_REG] = {
            /*DEVICE_ID = 0, */         3,
            /*DIAG_RESULT = 1,*/        1,
            /*OVER_TEMP = 2,*/          1,
            /*OC_DETECT = 3,*/          1,
            /*DEV_RESET = 4,*/          1,
            /*STANDBY = 5,*/            1,
            /*MODE = 6,*/               3,
            /*RTP_INPUT = 7,*/          8,
            /*HI_Z = 8,*/               1,
            /*LIBRARY_SEL2 = 9,*/       1,
            /*LIBRARY_SEL1 = 10,*/      1,
            /*LIBRARY_SEL0 = 11,*/      1,
            /*WAIT1 = 12,*/             1,
            /*WAV_FRM_SEQ1 = 13,*/      7,
            /*WAIT2 = 14,*/             1,
            /*WAV_FRM_SEQ2 = 15,*/      7,
            /*WAIT3 = 16,*/             1,
            /*WAV_FRM_SEQ3 = 17,*/      7,
            /*WAIT4 = 18,*/             1,
            /*WAV_FRM_SEQ4 = 19,*/      7,
            /*WAIT5 = 20,*/             1,
            /*WAV_FRM_SEQ5 = 21,*/      7,
            /*WAIT6 = 22,*/             1,
            /*WAV_FRM_SEQ6 = 23,*/      7,
            /*WAIT7 = 24,*/             1,
            /*WAV_FRM_SEQ7 = 25,*/      7,
            /*WAIT8 = 26,*/             1,
            /*WAV_FRM_SEQ8 = 27,*/      7,
            /*GO = 28,*/                1,
            /*ODT = 29,*/               8,
            /*SPT = 30,*/               8,
            /*SNT = 31,*/               8,
            /*BRT = 32,*/               8,
            /*ATH_PEAK_TIME = 33,*/     2,
            /*ATH_FILTER = 34,*/        2,
            /*ATH_MIN_INPUT = 35,*/     8,
            /*ATH_MAX_INPUT = 36,*/     8,
            /*ATH_MIN_DRIVE = 37,*/     8,
            /*ATH_MAX_DRIVE = 38,*/     8,
            /*RATED_VOLTAGE = 39,*/     8,
            /*OD_CLAMP = 40,*/          8,
            /*A_CAL_COMP = 41,*/        8,
            /*A_CAL_BEMF = 42,*/        8,
            /*N_ERM_LRA = 43,*/         1,
            /*FB_BRAKE_FACTOR = 44,*/   3,
            /*LOOP_GAIN = 45,*/         2,
            /*BEMF_GAIN = 46,*/         2,
            /*STARTUP_BOOST = 47,*/     1,
            /*AC_COUPLE = 48,*/         1,
            /*DRIVE_TIME = 49,*/        5,
            /*BIDIR_INPUT = 50,*/       1,
            /*BRAKE_STABILIZER = 51,*/  1,
            /*SAMPLE_TIME = 52,*/       2,
            /*BLANKING_TIME0 = 53,*/    2,
            /*IDISS_TIME0 = 54,*/       2,
            /*NG_THRESH = 55,*/         2,
            /*ERM_OPEN_LOOP = 56,*/     1,
            /*SUPPLY_COMP_DIS = 57,*/   1,
            /*DATA_FORMAT_RTP = 58,*/   1,
            /*LRA_DRIVE_MODE = 59,*/    1,
            /*N_PWM_ANALOG = 60,*/      1,
            /*LRA_OPEN_LOOP = 61,*/     1,
            /*ZC_DET_TIME = 62,*/       2,
            /*AUTO_CAL_TIME = 63,*/     2,
            /*OTP_STATUS = 64,*/        1,
            /*OTP_PROGRAM = 65,*/       1,
            /*AUTO_OL_CNT = 66,*/       2,
            /*LRA_AUTO_OPEN_LOOP = 67,*/1,
            /*PLAYBACK_INTERVAL = 68,*/ 1,
            /*BLANKING_TIME1 = 69,*/    2,
            /*IDISS_TIME1 = 70,*/       2,
            /*OL_LRA_PERIOD = 71,*/     7,
            /*VBAT = 72,*/              8,
            /*LRA_PERIOD = 73,*/        8,
        };

        //////////////
        /*Var region*/
        //////////////
        
        

        ///////////////
        /*Func region*/
        ///////////////

        //constructor
        /**
         * @brief Construct a new drv2605l tca object
         * @details default 
         */
        DRV2605L_TCA();

        /**
         * @brief Construct a new drv2605l tca object
         * 
         * @param EN_pin 
         * @param channel 
         */
        DRV2605L_TCA(int EN_pin, int channel);

        //destructor
        /**
         * @brief destructor drv2605l tca object
         * @details 
         * 1. 
         * 2. 
         * 
         */
        ~DRV2605L_TCA();

        //get private var 
        inline __attribute__((always_inline)) int get_EN(){return _EN;}
            
        inline __attribute__((always_inline)) int get_channel(){return _channel;}


        protected:
        ////////////////
        /*Const region*/
        ////////////////

        //////////////
        /*Var region*/
        //////////////
        Units _unit;
        Bias _bias;
        static inline uint8_t controlReg;

        ///////////////
        /*Func region*/
        ///////////////

        //TCA operation//

        /**
         * @brief select multi channel before communication
         * 
         */
        uint8_t TCA_selectMultiChannel(uint8_t ch);


        /**
         * @brief select specific channel before communication
         * 
         * @param ch from 0 to 7
         */
        uint8_t TCA_selectSingleChannel(uint8_t ch);

        /**
         * @brief 
         * 
         * @param val 
         */
        void setTCAControlReg(uint8_t val);

        /**
         * @brief 
         * 
         * @return uint8_t 
         */
        uint8_t getTCAControlReg();

        //bit operation//
        /**
         * @brief set bit pair (single regIndex) to specific value 
         * 
         * @param regIndex see DRV2605L_TCA::regIndex
         * @param val see DRV2605L_TCA::Val
         */
        void setBitPair(int regIndex, uint8_t val);

        /**
         * @brief get bit pair (single regIndex) and return 
         * 
         * @param regIndex see DRV2605L_TCA::regIndex
         * @return uint8_t , bit pair value
         */
        uint8_t getBitPair(int regIndex);

        /**
         * @brief inline version of #define GETMASK(L,sb) (((1 << L) - 1) << sb)
         * @details inline should be declared at .h file with static 
         * https://medium.com/@hauyang/%E6%88%91%E6%9C%89%E6%89%80%E4%B8%8D%E7%9F%A5%E7%9A%84-static-inline-b363892b7450
         * 
         * @param length 
         * @param startbit 
         * @return uint8_t , bitmask
         */
        static inline uint8_t getmask(uint8_t length, uint8_t startbit)
        {

        }


        private:
        ////////////////
        /*Const region*/
        ////////////////

        //////////////
        /*Var region*/
        //////////////
        int _channel = -1;
        int _EN = -1;
        
        ///////////////
        /*Func region*/
        ///////////////
        
    };
    
};

#endif


 