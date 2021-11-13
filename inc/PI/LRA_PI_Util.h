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
    }
#endif