#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <memory>
#include <atomic>
#include <vector>
#include <functional>
#include <mutex>

#include "noncopyable.hpp"
#include "timestamp.hpp"
#include "currentthread.hpp"
#include "timerqueue.hpp"

class Channel;
class Poller;
class EpollPoller;

class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();
    
    void loop();
    void quit();

    void wakeUp();
    void runInLoop(Functor);
    void queueInLoop(Functor);

    /*实现channel和poller之间的通信*/
    void updateChannel(Channel*);   
    void removeChannel(Channel*);
    bool hasChannel(Channel*);

    /*判断EventLoop是否在自己的线程中， threadId_是个线程全局变量，每个线程一份*/
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid();}

    /*添加删除定时器*/
    TimerId runAt(Timestamp time, const TimerCallback& cb);
    TimerId runAfter(double delay, const TimerCallback& cb);
    TimerId runEvery(double interval, const TimerCallback& cb);
    void cancel(TimerId timerId);

private:
    void handleRead();        /*wakeup的读事件处理函数*/
    void doPendingFunctor();  /*执行所有活跃的channel回调*/

private:
    using ChannelList = std::vector<Channel*>;

    pid_t threadId_;

    std::atomic_bool looping_;   /*正在执行循环*/
    std::atomic_bool quit_;      /*是否退出循环*/

    int wakeupFd_;                           /* 作为poller中的一个fd,用于唤醒当前loop中的poller*/
    std::unique_ptr<Channel> wakeupChannel_;
    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;


    ChannelList activeChannels_;        /*所有活跃的channel(有事件发生，等待执行回调函数的channel)*/
    Channel* currentActiveChannel_;     /*正在处理的channel*/
    bool eventHanding_;                 /*防止正在处理的channel被删除*/

    std::atomic_bool callingPendingFunctor_;   /*正在执行回调*/
    std::vector<Functor> pendingFunctor_;      /*存储所有进行*/
    std::mutex mutex_;                         /*用于保护上面的vector<Functor>线程安全*/
};

#endif