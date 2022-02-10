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
// thread
#include <thread>
#include <atomic>

#include <ErrorCode/LRA_ErrorCode.h>

#define GETMASK(length,startbit) (((1 << length) - 1) << startbit)
#define ABS(x) ((x ^ (x >> 31)) - (x >> 31))

namespace LRA_PI_Util{
    using namespace LRA_Error;
    using namespace fmt;
    using namespace std;

    #define I2C_DEFAULT_DEVICE "/dev/i2c-1"

    class Timer {
        //https://github.com/99x/timercpp
        std::atomic<bool> active{true};
        
        public:
            template<typename func>
            void setTimeout(func function, unsigned int udelay);

            template<typename func>
            void setInterval(func function, unsigned int uinterval);
            void stop();

    };

    /*timespec related*/
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