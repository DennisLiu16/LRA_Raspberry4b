#include <PI/LRA_PI_Util.h>
#include <string>

using namespace LRA_PI_Util;

/*class Timer*/
template <typename func>
void Timer::setTimeout(func function, unsigned int udelay) {
    active = true;
    std::thread t([=]() {
        if(!active.load()) return;
        std::this_thread::sleep_for(std::chrono::microseconds(udelay));
        if(!active.load()) return;
        (*function)();  // how your function work 
    });
    t.detach();
}

template <typename func>
void Timer::setInterval(func function, unsigned int uinterval) {
    active = true;
    std::thread t([=]() {
        while(active.load()) {
            std::this_thread::sleep_for(std::chrono::microseconds(uinterval));
            if(!active.load()) return;
            (*function)();  // how your function work 
        }
    });
    t.detach();
}

void Timer::stop() {
    active = false;
}

// define instantiate
template void Timer::setTimeout<void(*)()>(void(*function)(), unsigned int udelay); // important
template void Timer::setInterval<void(*)()>(void(*function)(), unsigned int uinterval);