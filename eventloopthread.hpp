#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H
#include "thread.hpp"
#include "noncopyable.hpp"

#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>


class EventLoop;

class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& callback = ThreadInitCallback()
                , const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();

private:
    EventLoop *loop_;
    std::mutex mutex_;
    std::condition_variable cond_;


    bool exiting_; /*创建的loop线程是否存在*/
    Thread thread_;
    ThreadInitCallback callback_;
};

#endif