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
#include <inc/LRA_ErrorCode.h>
#include <inc/LRA_PI_Util.h>

namespace LRA_DRV2605L{
    using namespace LRA_PI_Util;
    using namespace LRA_Error;
    
    /*I2C related*/
    #define I2C_DEFAULT_TARGET "/dev/i2c-1"
    
    /*Address*/
    typedef enum{
        ADDR_DEFAULT = 0x5a,
        ADDR_X       = 0x71,
        ADDR_Y       = 0x72,
        ADDR_Z       = 0x73,
    }ADDR;

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
        REG_LRAResonancePeriod                  = 0x21,
    }REGISTER;



    /*class*/
    class DRV2605L{
        public:
            int slave_id;
            uint16_t errCode;
            uint16_t read(int slave_id, int reg_addr);
            uint8_t write(int slave_id, int reg_addr);
           
        protected:
            PI_I2C i2c;
            Data data;

        private:
    };
}
#endif