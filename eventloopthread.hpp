#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include "thread.hpp"
#include "noncopyable.hpp"

#include <string>
#include <functional>
#include <condition_variable>
#include <mutex>

class EventLoop;

class EventLoopThread
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThread(const ThreadInitCallback& cb, const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();

private:
    ThreadInitCallback threadInitCallback_;  //每个线程初始化马上调用threadInitCallback_函数
    EventLoop* loop_;
    std::condition_variable cond_;
    std::mutex mutex_;

    Thread thread_;
    bool exiting_;
};

#endif