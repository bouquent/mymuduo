 #include "eventloopthreadpool.hpp"
 #include "eventloopthread.hpp" 
 
 #include <memory>
 
EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& name)
    : baseLoop_(baseLoop)
    , name_(name)
    , started_(false)
    , numThreads_(0)
    , next_(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{}


void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        char subname[32];
        snprintf(subname, 32, "%s: %d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, subname);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        
        loops_.push_back(t->startLoop());
    }

    if (numThreads_ == 0 && cb) {
        /*没有subloop, 让mainloop去执行回调函数*/
        cb(baseLoop_);
    }
}


EventLoop* EventLoopThreadPool::getNextLoop()
{
    /*如果没有子线程，所有任务主线程担当*/
    EventLoop *nextLoop = baseLoop_; 
    if (!loops_.empty()) {
        nextLoop = loops_[next_++];  
        if (loops_.size() == static_cast<std::size_t>(next_)) {  
            /*所有的loop都被派发了一次任务，回到第一个loop再开始派发*/
            next_ = 0;
        }
    }
    return nextLoop;
}