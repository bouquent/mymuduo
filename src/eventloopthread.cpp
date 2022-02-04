#include "eventloopthread.hpp"
#include "thread.hpp"
#include "logging.hpp"
#include "eventloop.hpp"

#include <functional>
#include <memory>

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string& name)
                        : threadInitCallback_(cb)
                        , loop_(nullptr)
                        , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
                        , exiting_(false)
{}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start();

    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    
    if (threadInitCallback_) {
        threadInitCallback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_all();
    }

    loop_->loop();

    //如果出loop函数，说明这个线程即将结束了
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}