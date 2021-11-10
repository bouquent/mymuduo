#ifndef THREAD_H
#define THREAD_H

#include "noncopyable.hpp"

#include <functional>
#include <memory>
#include <thread>
#include <string>
#include <atomic>

class Thread : noncopyable
{
public:
    using ThreadFunc = std::function<void()>;
    explicit Thread(ThreadFunc func, const std::string& name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    bool joined() const { return joined_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }
    static int numCreated() { return numCreated_; }

private:
    /*获取默认名字*/
    void setDefaultName();


    std::shared_ptr<std::thread> thread_;
    ThreadFunc func_;                      /*线程该执行的函数*/
    std::string name_;
    static std::atomic_int numCreated_;   /*第几个被创建的线程*/


    pid_t tid_;      /*这个线程启动的用于loop的线程*/
    bool started_;   /*loop线程是否启动*/
    bool joined_;    /*loop线程是否被回收*/
};

#endif 