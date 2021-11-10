#include "timerqueue.hpp"
#include "eventloop.hpp"
#include "logging.hpp"

#include <sys/timerfd.h>
#include <functional>
#include <unistd.h>
#include <strings.h>
#include <vector>

time_t howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.mircoseconds() - Timestamp::now().mircoseconds();

    if (microseconds < 100) {
        microseconds = 100;
    }
    return microseconds;
}
/*创建一个timerfd*/
int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (timerfd < 0) {
        LOG_FATAL("[%s]:%s timerfd_create wrong!", __FILE__, __func__);
    }
    return timerfd;
}
/*读取timerfd中的数据*/
void readTimerfd(int timerfd, Timestamp now)
{
	uint64_t howmany;
	ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
	if (n != sizeof(howmany)) {
		LOG_ERROR("[%s]:%s read wrong!\n", __FILE__, __func__);
	}
}

/*重置定时器超时时刻*/
void resetTimerfd(int timerfd, Timestamp expiration)
{
	struct itimerspec newValue;
	struct itimerspec oldValue;
	bzero(&newValue, sizeof(newValue));
	bzero(&oldValue, sizeof(oldValue));
	newValue.it_value.tv_sec = howMuchTimeFromNow(expiration);
    
    /*到期后timerfd会发出epollin事件*/
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        LOG_ERROR("[%s]:%s timerfd_settime wrong", __FILE__, __func__);
    }

}

TimerQueue::TimerQueue(EventLoop *loop)
            : loop_(loop)
            , timerfd_(createTimerfd())
            , timerfdChannel_(loop, timerfd_)
            , timers_()
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
    /*是否需要更改定时探测时间*/
    bool earliestChanged = insert(timer);
    
    if (earliestChanged) {
        /*有更早的定时任务，需要更改超时时间*/
        resetTimerfd(timerfd_, timer->expiration());
    } 
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;   /*最早的到期时间是否改变*/

    Timestamp when = timer->expiration();
    auto it = timers_.begin();

    if (it == timers_.end() || when < it->first) {
        earliestChanged = true;
    }

    timers_.insert(Entry(when, timer));
    /*插入到activeTimers中*/
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
        /*找到了这个定时器任务*/
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        delete it->first;
        activeTimers_.erase(it);
    } else if (callingExpiredTimers_) {
        /*可能这个定时器正在处理*/
        cancelingTimers_.insert(timer);
    }

}


/*可读事件处理*/
void TimerQueue::handleRead()
{
	Timestamp now(Timestamp::now());
	/*读取该事件缓冲区数据，避免多次触发*/
	readTimerfd(timerfd_, now);

	std::vector<Entry> expired = getExpired(now);
	callingExpiredTimers_ = true;
	cancelingTimers_.clear();  /*清空之前被处理过的定时任务*/
	for (auto iter = expired.begin(); iter != expired.end(); iter++) {
		iter->second->run();
	}
	callingExpiredTimers_ = false;

	reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sendtry(now, nullptr);

    auto end = timers_.lower_bound(sendtry);
    std::copy(timers_.begin(), end, back_inserter(expired));
    timers_.erase(timers_.begin(), end);  //删除所有已到期的定时器

    /*将activeTimers_中删除所有到期的定时器*/
    for (auto iter = expired.begin(); iter != expired.end(); ++ iter) {
        ActiveTimer timer(iter->second, iter->second->sequence());
        size_t n = activeTimers_.erase(timer);
    }
    
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;

    for (auto iter = expired.begin(); iter != expired.end(); ++iter) {
        ActiveTimer timer(iter->second, iter->second->sequence());
        if (iter->second->repeat() && cancelingTimers_.find(timer) == cancelingTimers_.end()) {
            iter->second->restart(now);
            insert(iter->second);
        } else {
            delete iter->second;
        }

        if (!timers_.empty()) {
            nextExpire = timers_.begin()->second->expiration();
        }

        /*设置下一个定时触发时间*/
        if (nextExpire.vaild()) {
            resetTimerfd(timerfd_, nextExpire);
        }
    }
}