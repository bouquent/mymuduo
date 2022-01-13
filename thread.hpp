#ifndef THREAD_H
#define THREAD_H

#include "noncopyable.hpp"
#include "currentthread.hpp"

#include <unistd.h>
#include <functional>
#include <atomic>
#include <memory>
#include <thread>
#include <string>

class Thread : noncopyable
{
public:
    using ThreadCallback = std::function<void()>;   
    Thread(const ThreadCallback& cb, const std::string& name = std::string());
    ~Thread();

    void start();
    void join();


    std::string name() const { return name_; }
    bool joined() const { return joined_; }
    pid_t tid() const { return tid_; }
private:
    void setDefaultName();

private:
    ThreadCallback threadfunc_;
    std::string name_;
    std::shared_ptr<std::thread> thread_;
    bool joined_; 
    bool started_;
    pid_t tid_;

    static std::atomic_int createdNum;
};


#endif 