#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOLL_H

#include "noncopyable.hpp"

#include <functional>
#include <vector>
#include <string>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPool(EventLoop* baseLoop, const std::string& name = std::string());
    ~EventLoopThreadPool();

    void start(const ThreadInitCallback& cb);
    EventLoop* getNextLoop();

    //设置线程(subloop)数量，如果为0则只有一个mainloop在执行服务
    void setThreadNum(int threadNums) {threadNums_ = threadNums; }
    int getThreadNum() const { return threadNums_; }
    std::string name() const { return name_; }
    const std::vector<EventLoop*> getAllLoop() const { return loops_; }
private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
    int next_;
    int threadNums_;
};

#endif