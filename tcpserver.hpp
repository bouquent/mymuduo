#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "inetaddr.hpp"
#include "callbacks.hpp"
#include "acceptor.hpp"
#include "eventloopthreadpool.hpp"
#include "noncopyable.hpp"
#include "tcpconnection.hpp"
#include "timestamp.hpp"
#include "buffer.hpp"

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <atomic>

class EventLoop;

class TcpServer : noncopyable
{
public:
    enum Option
    {
        kNoReusePort,
        kReusePort,
    };
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    
    TcpServer(EventLoop *loop, 
            const InetAddr& addr, 
            const std::string& nameArg, 
            Option option = kReusePort);
    ~TcpServer();


    /*启动服务*/
    void start();

    /*设置线程数量(subloop)*/
    void setThreadNum(int threadNums);

    /*处理用户的连接断开*/
    void newConnection(int sockfd, const InetAddr &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    /*设置回调函数*/
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = std::move(cb); }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = std::move(cb); }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = std::move(cb); }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = std::move(cb); }

    std::string ipPort() const { return ipPort_; }
    std::string name() const { return name_; }
private:
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
    ConnectionMap connections_;
    int nextConnId_;

    std::atomic_int32_t started_; /*防止服务器被启动多次*/
    EventLoop *baseLoop_;
    const std::string name_;
    const std::string ipPort_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    MessageCallback messageCallback_;

    ThreadInitCallback threadInitCallback_;    
};


#endif