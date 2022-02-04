#ifndef TIMERLD_H
#define TIMERLD_H

#include "copyable.hpp"
#include "timer.hpp"

class TimerId : public copyable
{
public:
    TimerId()
        : timer_(nullptr)
        , sequence_(0)
    {}
    TimerId(Timer* tm, int64_t seq)
        : timer_(tm)
        , sequence_(seq)
    {}


    friend class TimerQueue;
private:
    Timer *timer_;
    int64_t sequence_;
};

#endif
