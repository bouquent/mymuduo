#include "thread.hpp"
#include "currentthread.hpp"
#include <iostream>
#include <semaphore.h>

std::atomic_int Thread::numCreated_(0);

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if (name_.empty()) {
        /*如果没有给定特定的名字,给这个线程设置一个默认的name*/
        char buf[32];
        snprintf(buf, 32, "Thread:%d", num);
        name_ = buf;
    }
}

Thread::Thread(ThreadFunc func, const std::string &name)
    : func_(std::move(func))
    , name_(name)
    , tid_(0)
    , started_(false)
    , joined_(false)
{
    setDefaultName();
}

Thread::~Thread()
{
    if (started_ && ! joined_) {
        thread_->detach();
    }
}


void Thread::start()
{
    sem_t sem;
    sem_init(&sem, false, 0);
    
    started_ = true;
    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();
    }));

    /*保证tid_被赋值才退出函数*/
    sem_wait(&sem); 
}

void Thread::join()
{
    joined_ = true;
    thread_->join();
}