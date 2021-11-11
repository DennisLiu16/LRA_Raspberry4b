#ifndef _LRA_PI_UTIL_H_
#define _LRA_PI_UTIL_H_

// sys include - c++
#include <iostream>
#include <string>
// sys include - c
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>

#include <ErrorCode/LRA_ErrorCode.h>

    namespace LRA_PI_Util{
        using namespace LRA_Error;
        using namespace std;

        #define I2C_DEFAULT_DEVICE "/dev/i2c-1"
        

        class Data{
            public:
                Data();
                ~Data();
                /**
                 * @brief need to redeside param
                 * 
                 * @param file_name 
                 * @return uint8_t 
                 */
                uint8_t store_as_csv(char* file_name);
                /**
                 * @brief need to redeside param
                 * 
                 * @param table_name 
                 * @return uint8_t 
                 */
                uint8_t store_as_database(char* table_name);

            protected:

            private:

        };

        class PI_I2C{
            public:
                PI_I2C();
                PI_I2C(int slave_id);
                ~PI_I2C();

                int slave_id;
                int* i2c_port = nullptr;           // file_i2c, the channel
                uint16_t err;
                Data data;

                

                int* i2c_init(char* dev_id = (char*)I2C_DEFAULT_DEVICE) noexcept;
                /**
                 * @param   :   (char*) ,i2c_dev_id default as "/dev/i2c-1"
                 * @return  :   (int)   ,i2c_interface, read/write through this port
                 * @note    :   noexcept means if open i2c failed will get in exception dealing -> abort
                 */ 
                uint8_t i2c_read(int* port,int reg_addr);
                /**
                 * @brief need to change
                 * @param   :   (int,int)
                 * @return  :   (uint8_t)  one byte data
                 * @note    :   if address exception deal in LRA_DRV2605L.cpp, but deal slave id error here
                 */
                uint8_t i2c_write(int* port,int reg_addr, uint8_t content);
                /**
                 * @brief need to change
                 * @param   :   (int,int)
                 * @return  :   (int) error code
                 * @note    :   if address exception deal in LRA_DRV2605L.cpp, but deal slave id error here
                 */

            protected:
                

            private:
        };

        
    }
#endif