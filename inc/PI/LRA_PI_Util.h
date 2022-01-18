#ifndef _LRA_PI_UTIL_H_
#define _LRA_PI_UTIL_H_
/*essential define or ompile will error*/
#define FMT_HEADER_ONLY

// sys include - c++
#include <iostream>
#include <string>
// sys include - c
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/color.h>
#include <sys/time.h>

#include <ErrorCode/LRA_ErrorCode.h>

    namespace LRA_PI_Util{
        using namespace LRA_Error;
        using namespace fmt;
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

        /**
         * @brief 
         * 
         * @param ts 
         * @param te 
         * @return double (ms)
         */
        double static inline time_diff_ms(timespec* ts, timespec* te)
        {
            return ((te->tv_sec - ts->tv_sec) * 1e3 +  (te->tv_nsec - ts->tv_nsec) * 1e-6);
        }

        double static inline time_diff_us(timespec* ts,timespec* te)
        {
            return ((te->tv_sec - ts->tv_sec) * 1e6 +  (te->tv_nsec - ts->tv_nsec) * 1e-3);
        }

        double static inline time_diff_ns(timespec* ts,timespec* te)
        {
            return ((te->tv_sec - ts->tv_sec) * 1e9 +  te->tv_nsec - ts->tv_nsec);
        }
        
    }
#endif