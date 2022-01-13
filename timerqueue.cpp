#include "timerqueue.hpp"
#include "eventloop.hpp"
#include "logging.hpp"

#include <sys/timerfd.h>
#include <functional>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <vector>
#include <iterator>


timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.mircoseconds() - Timestamp::now().mircoseconds();

    if (microseconds < 100) {
        microseconds = 100;
    }

    timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kmircoSecondsPreSecond);
    ts.tv_nsec = static_cast<time_t>(microseconds % Timestamp::kmircoSecondsPreSecond * 1000);
    return ts;
}
/*创建一个timerfd*/
int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (timerfd < 0) {
        LOG_FATAL("[%s]:%s timefd_create wrong!", __FILE__, __func__);
    }
    return timerfd;
}

/*重置定时器超时时刻*/
void resetTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    memset(&newValue, 0, sizeof(newValue));
    memset(&oldValue, 0, sizeof(oldValue));

    //计算定时的时间
    newValue.it_value = howMuchTimeFromNow(expiration); 
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret < 0) {
        LOG_FATAL ("[%s]:%s timerfd_settime wrong!", __FILE__, __func__);
    }
}

/*读取timerfd中的数据*/
void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    int n = ::read(timerfd, &howmany, sizeof(howmany));
    if (n != sizeof(howmany)) {
        LOG_ERROR("[%s]:%s read wrong!", __FILE__, __func__);
    }
}



TimerQueue::TimerQueue(EventLoop *loop)
                    : loop_(loop)
                    , timerfd_(createTimerfd())
                    , timerfdChannel_(loop, timerfd_)
                    , callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));

    timerfdChannel_.enableReading();
}


TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);

    for (auto iter = timers_.begin(); iter != timers_.begin(); iter++) {
        delete iter->second;
    }
}


/*增加一个定时器*/
TimerId TimerQueue::addTimer(const TimerCallback& cb, 
                            Timestamp when, 
                            double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));

    return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer *timer)
{
    //是否需要更改定时时间（是否有比最小早定时器还早的定时器加入了其中）
    bool earlistestChanged = insert(timer);

    if (earlistestChanged) {
        resetTimerfd(timerfd_, timer->expiration());
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->expiration();

    auto it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }

    //加入到timerqueue中
    timers_.insert(Entry(when, timer));
    activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
 
    return earliestChanged;
}



void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(std::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(TimerId timerId)
{
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    auto it = activeTimers_.find(timer);

    if (it != activeTimers_.end()) {
        timers_.erase(Entry(it->first->expiration(), it->first));
        delete it->first;
        activeTimers_.erase(it);
    } else if (callingExpiredTimers_) {
        //这个定时器到期了但可能正在执行回调函数
        cancelingTimers_.insert(timer);
    }
}



/*可读事件处理*/
void TimerQueue::handleRead()
{
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_, now);
    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = false;
    //清空之前被处理过的定时任务
    cancelingTimers_.clear();
    for (auto iter = expired.begin(); iter != expired.end(); ++iter) {
        iter->second->run();
    }
    callingExpiredTimers_ = true;

    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sendtry(now, nullptr);

    //end是第一个晚于now的定时器
    auto end = timers_.lower_bound(sendtry);
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);  //删除所有已到期的定时器

    //将activeTimers_中删除所有到期的定时器
    for (auto iter = expired.begin(); iter != expired.end(); ++iter) {
        ActiveTimer timer(iter->second, iter->second->sequence());
        activeTimers_.erase(timer);
    }

    return expired;
}


void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{   
    for (auto iter = expired.begin(); iter != expired.end(); ++iter) {
        ActiveTimer timer(iter->second, iter->second->sequence());
        if (iter->second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            iter->second->restart(now);
            insert(iter->second);
        } else {
            delete iter->second;
        }
    }

    Timestamp nextExpired;
    if (!timers_.empty()) {
        nextExpired = timers_.begin()->second->expiration();
        resetTimerfd(timerfd_, nextExpired);
    }
}