#ifndef TIMER_H
#define TIMER_H

#include "noncopyable.hpp"
#include "timestamp.hpp"
#include "callbacks.hpp"
#include <atomic>

class Timer : noncopyable
{
public:
    Timer(TimerCallback cb, Timestamp when, double interval)
        : callback_(std::move(cb))
        , expiration_(when)
        , interval_(interval)
        , repeat_(interval > 0.0)
        , sequence_(s_numCreated_++)
    {}

    /*定时器回调函数*/
    void run() const 
    {
        callback_();
    }

    Timestamp expiration() const { return expiration_; }
    bool repeat() const { return repeat_; }
    int64_t sequence() const { return sequence_; }

    void restart(Timestamp now);

    static int64_t s_numCreated() { return s_numCreated_; }
private:
    TimerCallback callback_;
    Timestamp expiration_;    /*下一次的超时时刻*/
    const double interval_;   /*超时时间间隔*/
    const bool repeat_;       /*是否重复*/
    const int64_t sequence_;    /*定时器序号*/

    static std::atomic_int32_t s_numCreated_;
};

#endif
