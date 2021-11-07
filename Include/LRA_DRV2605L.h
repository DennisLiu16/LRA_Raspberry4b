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

namespace LRA_DRV2605L{
    /*I2C related*/
    #define I2C_DEFAULT_TARGET "/dev/i2c-1"

    /*Address*/
    typedef enum{
        ADDR_DEFAULT = 0x5a,
        ADDR_X       = 0x01,
        ADDR_Y       = 0x02,
        ADDR_Z       = 0x03,
    }ADDR;

    /*Register*/
    typedef enum{

    }REGISTER;

    /*class*/
    class DRV2605L{
        public:

        protected:

        private:
    };
}
#endif