#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOLL_H

#include "noncopyable.hpp"

#include <functional>
#include <string>
#include <vector>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name = std::string());
    ~EventLoopThreadPool();

    void start(const ThreadInitCallback& cb = ThreadInitCallback());
    EventLoop* getNextLoop();

    /*设置线程(subloop)数量，如果为0则只有一个mainloop在执行服务*/
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }

    bool started() const { return started_; }
    const std::string& name() const { return name_; }
    std::vector<EventLoop*> getAllLoops() const { return loops_; }
private:
    ThreadInitCallback callback_;

    EventLoop *baseLoop_;   /*main subloop*/
    std::string name_;
    bool started_;      /*是否创建了子线程(subloop)*/  
    int numThreads_;    /*subloop的数量*/
    int next_;          /*下一个接受服务用户的loop*/

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};


#endif