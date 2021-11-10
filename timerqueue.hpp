#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include "timer.hpp"
#include "timerid.hpp"
#include "channel.hpp"
#include "noncopyable.hpp"
#include "timestamp.hpp"

#include <set>
#include <vector>
#include <utility>
class EventLoop;

class TimerQueue : noncopyable
{
public:
    TimerQueue(EventLoop *loop);
    ~TimerQueue();

    TimerId addTimer(const TimerCallback &cb, 
                    Timestamp when,
                    double interval);
        
    void cancel(TimerId timerId);

private:
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;
    
    void addTimerInLoop(Timer *timer);
    void cancelInLoop(TimerId timerId);

    void handleRead();    /*定时器时间产生的回调函数*/

    std::vector<Entry> getExpired(Timestamp now); /*返回超时的定时任务*/

    void reset(const std::vector<Entry>& expired, Timestamp now); /*对超时的定时器进行重置，因为超时的定时器可能是重复的定时器*/

    bool insert(Timer *timer);  /*插入定时器*/
private:
    EventLoop *loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    TimerList timers_;              /*按时间戳进行排序*/

    ActiveTimerSet activeTimers_;   /*活跃定时器列表，按地址进行排序*/
    bool callingExpiredTimers_;
    ActiveTimerSet cancelingTimers_;/*被取消的定时器*/
};


#endif