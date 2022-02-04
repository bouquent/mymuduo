#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include "timerid.hpp"
#include "timer.hpp"
#include "noncopyable.hpp"
#include "channel.hpp"

#include <set>
#include <vector>

class EventLoop;

class TimerQueue : noncopyable 
{
public:
    explicit TimerQueue(EventLoop*);
    ~TimerQueue();

    TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);
    void cancel(TimerId timerId);
private:
    using Entry = std::pair<Timestamp, Timer*>;  //时间， 定时器
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, int64_t>; // 定时器，序号
    using ActiveTimerSet = std::set<ActiveTimer>;

    bool insert(Timer*);            //返回bool类型 主要是为了判断是否需要重新设置定时超时的时间
    void addTimerInLoop(Timer*);
    void cancelInLoop(TimerId timerId);

    void handleRead();                               //取出timerfd中的uint64_t，以免timerfd描述符一直触发事件
    std::vector<Entry> getExpired(Timestamp now);    //返回超时的定时器
    void reset(const std::vector<Entry>& expired, Timestamp now);

private:
    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;

    TimerList timers_;
    ActiveTimerSet activeTimers_;
    ActiveTimerSet cancelingTimers_;
    bool callingExpiredTimers_;
};



#endif