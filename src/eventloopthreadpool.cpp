#include "eventloopthreadpool.hpp"
#include "eventloopthread.hpp" 
 
 #include <memory>
 

 //主要就是两个任务  1 选择下一个subloop， 根据threadnums创建eventloopthread

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& name)
                                    : baseLoop_(baseLoop)
                                    , name_(name)
                                    , started_(false)
                                    , next_(0)
                                    , threadNums_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{}

void EventLoopThreadPool::start(const ThreadInitCallback& threadInitCallback_)
{
    started_ = true;

    if (threadNums_ == 0 && threadInitCallback_) {
        //如果没有subloop，回调函数由mainloop调用
        threadInitCallback_(baseLoop_);
    }

    for (int i = 0; i < threadNums_; ++i) {
        char subname[32] = {0};
        snprintf(subname, 32, "%s:%d", name_.c_str(), i);

        EventLoopThread *p = new EventLoopThread(threadInitCallback_, subname);
        threads_.push_back(std::unique_ptr<EventLoopThread>(p));

        loops_.push_back(p->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
     //如果没有subloop，所有任务都有主线程担任
    if (loops_.empty()) {
        return baseLoop_;
    }
    
    //选取下一个subloop
    EventLoop *nextLoop = loops_[next_++];
    if (static_cast<std::size_t>(next_) == loops_.size()) {
        next_ = 0;
    }
    return nextLoop;
}


