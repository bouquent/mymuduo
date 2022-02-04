#include <sys/eventfd.h>
#include <stdlib.h>
#include <unistd.h>
#include <functional>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <mutex>

#include "eventloop.hpp"
#include "poller.hpp"
#include "channel.hpp"
#include "logging.hpp"
#include "currentthread.hpp"

__thread EventLoop *t_loopInThisThread = nullptr;  /*防止一个thread创建多个loop*/
const int kPollTimeMs = 10000;


int createEventFd()
{
    int wakeupFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeupFd < 0) {
        LOG_FATAL("[%s]:%s evenfd is error", __FILE__, __func__);
    }
    return wakeupFd;
}

//向wakeupFd写数据,唤醒阻塞在poll的线程去执行回调函数
void EventLoop::wakeUp()
{
    uint64_t howmany = 1;
    int ret = write(wakeupFd_, &howmany, sizeof(uint64_t));
    if (ret == -1 && errno != EINTR) {
        LOG_ERROR("[%s]:[%s] wakeupFd write error, errno is %d", __FILE__, __func__, errno);
    } 
}

/*wakeup的读事件处理函数,将用来唤醒写入的一个字节读取*/
void EventLoop::handleRead()
{
    uint64_t howmany;
    int ret = read(wakeupFd_, &howmany, sizeof(uint64_t));
    if (ret != sizeof(uint64_t)) {
        LOG_ERROR("[%s]:[%s] wakeupFd read error, errno is %d", __FILE__, __func__, errno);
    }
}


EventLoop::EventLoop()
    : threadId_(CurrentThread::tid())  
    , looping_(false)
    , quit_(false)
    , wakeupFd_(createEventFd())
    , wakeupChannel_(new Channel(this, wakeupFd_))
    , poller_(Poller::newDefaultPoller(this, Poller::EPOLL_POLLER))
    , timerQueue_(new TimerQueue(this))
    , currentActiveChannel_(nullptr)
    , eventHanding_(false)
    , callingPendingFunctor_(false)
{
    if (t_loopInThisThread == nullptr) {
        t_loopInThisThread = this;
    } else {
        LOG_FATAL("[%s]: %s thread create loop agina", __FILE__, __func__);
    }

    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    looping_ = true;
    while (!quit_) {
        activeChannels_.clear();    /*将上一次处理的任务清空*/
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        eventHanding_ = true;
        for (auto channel : activeChannels_) {
            /*执行每个活跃的channel的回调函数(客户层面的业务)*/
            currentActiveChannel_ = channel;
            channel->handleEvent(pollReturnTime_);
        }
        eventHanding_ = false;
        currentActiveChannel_ = nullptr;

        /*执行EventLoop层面的业务回调*/
        doPendingFunctor();
    }
    LOG_DEBUG("EventLoop %p stop looping!", this);
    looping_ = false;
}


void EventLoop::doPendingFunctor()  
{
    std::vector<Functor> functors;
    callingPendingFunctor_ = true;
    {
        std::lock_guard<std::mutex> guard(mutex_);
        /*快速获取pendingFunctor的回调并且置空*/
        functors.swap(pendingFunctor_);
    }
    for (auto functor : functors) {
        functor();
    }
    callingPendingFunctor_ = false;
}



void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread()) {
        /*在别的线程中调用的这个EventLoop,唤醒阻塞在epoll_wait上的线程,让它快速结束整个loop循环*/
        this->wakeUp();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if (isInLoopThread()) {
        /*操作loop的是当前线程，直接执行回调*/
        cb(); 
    } else {
        /*操作loop的不是当前线程，加入这个loop的等待执行队列中*/
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex> guard(mutex_);
        pendingFunctor_.push_back(cb);
    }
    if (!isInLoopThread() || callingPendingFunctor_ || eventHanding_)
    {
        /*1. 不在当前线程中调用loop，唤醒阻塞在epoll_wait中的线程，去执行回调
        * 2. 正在执行functor回调或者channel回调,该线程执行这一次后，下一次epoll_wait不阻塞直接去执行回调
        *  都是为了让回调操作快速被执行
        */
       this->wakeUp();
    }
}

/*实现channel和poller之间的通信*/
void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}  
void EventLoop::removeChannel(Channel* channel)
{
    while (eventHanding_) {
        LOG_ERROR("remove channel(fd is %d), but it runing", channel->fd());
        sleep(1);   //等待该channel的事件处理完
    }
    poller_->removeChannel(channel);
}
bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}


TimerId EventLoop::runAt(Timestamp time, const TimerCallback& cb)
{
    return timerQueue_->addTimer(cb, time, 0.0);
}
TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}
TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}
void EventLoop::cancel(TimerId timerId)
{
    timerQueue_->cancel(timerId);
}

