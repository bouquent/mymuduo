#include "timer.hpp"

std::atomic_int32_t Timer::s_numCreated_(0);

void Timer::restart(Timestamp now)
{
    if (repeat_) {
        expiration_ = addTime(now, interval_);
    } else {
        expiration_ = Timestamp::invaild();
    }
}