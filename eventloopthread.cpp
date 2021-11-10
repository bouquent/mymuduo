#include "eventloopthread.hpp"
#include "thread.hpp"
#include "logging.hpp"
#include "eventloop.hpp"

#include <functional>
#include <memory>

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name)
    : callback_(cb)
    , exiting_(true)
    , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
    , loop_(nullptr)
    , mutex_()
    , cond_()
{}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr) { 
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start();

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (nullptr == loop_) {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    if (callback_) {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();

    /*如果loop函数执行结束，说明这个loop以及对应的Poller即将结束*/
    std::lock_guard<std::mutex> guard(mutex_);
    loop_ = nullptr;
}