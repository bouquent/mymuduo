#ifndef TIMER_H
#define TIMER_H

#include "noncopyable.hpp"
#include "timestamp.hpp"
#include "callbacks.hpp"
#include <atomic>

class Timer : noncopyable
{
public:
    explicit Timer(const TimerCallback& cb, Timestamp when, double interval)
                : callback_(cb)
                , expiration_(when)
                , interval_(interval)
                , repeat_(interval > 0.0)
                , sequence_(s_numCreated_++)
    {}

    Timestamp expiration() const { return expiration_; }
    double interval() const { return interval_; }
    bool repeat() const { return repeat_; }
    uint64_t sequence() const { return sequence_; }

    void run()
    {
        callback_();
    }
    void restart(Timestamp now);
private:
    TimerCallback callback_;
    Timestamp expiration_;     //超时时刻
    const double interval_;    //超时时间间隔
    const bool repeat_;        //是否重复
    const uint64_t sequence_;  //定时器序号

    static std::atomic_uint64_t s_numCreated_;
};


#endif
