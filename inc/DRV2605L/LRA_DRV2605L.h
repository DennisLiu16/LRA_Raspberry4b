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

        // Setting binary code

    }REGISTER;

    uint8_t Default_Value[REG_NUM] = {
        0xe0, 0x40, 0x0,  0x1,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 
        0x0,  0x05, 0x19, 0xff, 0x19, 0xff, 0x3e, 0x8c, 0x0c, 0x6c, 0x36, 0x93, 0xf5, 0xa0, 0x20, 0x80,
        0x33, 0x0 , 0x0
    };

    uint8_t LRA_Setting[REG_NUM] = {
        /**
         * 0x00 :
         * 0x01 :
         * 0x02 :
         * 0x03 :
         * 0x04 :
         * 0x05 :
         * 0x06 :
         * 0x07 :
         * 0x08 :
         * 0x09 :
         * 0x0A :
         * 0x0B :
         * 0x0C :
         * 0x0D :
         * 0x0E :
         * 0x0F :
         * 0x10 :
         * 0x11 :
         * 0x12 :
         * 0x13 :
         * 0x14 :
         * 0x15 :
         * 0x16 :
         * 0x17 :
         * 0x18 :
         * 0x19 :
         * 0x1A :
         * 0x1B :
         * 0x1C :
         * 0x1D :
         * 0x1E :
         * 0x1F :
         * 0x20 :
         * 0x21 :
         * 0x22 :
         */
        0xe0, 0x40, 0x0,  0x1,  0x1,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 
        0x0,  0x05, 0x19, 0xff, 0x19, 0xff, 0x3e, 0x8c, 0x0c, 0x6c, 0x36, 0x93, 0xf5, 0xa0, 0x20, 0x80,
        0x33, 0x0 , 0x0
    };

    /*class*/
    class DRV2605L{
        public:
            DRV2605L();
            DRV2605L(int slave_id = SLAVE_DEFAULT_ID);
            ~DRV2605L();

            bool is_init = 0;
            int slave_id;
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

            ssize_t write(uint32_t reg_addr, uint8_t content);

        protected:
            /*return value PI_I2C 's func pointer i2c  v.s. create a var 's type is PI_I2C*/
            I2CDevice i2c;
            Data data;

        private:
    };
}
#endif