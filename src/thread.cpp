#include "thread.hpp"
#include "currentthread.hpp"
#include <iostream>

std::atomic_int Thread::createdNum(0);

void Thread::setDefaultName()
{
    int num = Thread::createdNum++;
    if (name_.empty()) {
        //如果没有名字，那么给这个线程设置一个默认的名字
        char name[32] = {0};
        snprintf(name, 32, "Thread:%d", num);
        name_ = name;
    }
}

Thread::Thread(const ThreadCallback& cb, const std::string& name)
            : threadfunc_(cb)
            , name_(name)
            , joined_(false)
            , started_(false)
            , tid_(0)       //此时还在主线程，不能给还没有创建的线程的tid赋值
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && !joined_) {
        thread_->detach();
    }
}


void Thread::start()
{
    started_ = true;
    
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        tid_ = CurrentThread::tid();
        threadfunc_();
    }));

    //等tid_被赋值后才推出，不然tid还是0
    while (0 == tid_) {}
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}

