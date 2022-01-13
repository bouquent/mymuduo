#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "noncopyable.hpp"
#include "inetaddr.hpp"
#include "socket.hpp"
#include "buffer.hpp"
#include "callbacks.hpp"
#include "timestamp.hpp"
#include "eventloop.hpp"

#include <memory>
#include <string>
#include <atomic>

/*  Acceptor接受到进用户后，拿到connfd
*   创造TcpConnection 并设置回调函数(这个时候channel对应的loop已经确定了)，注册channel,并给poller进行监听
*   发生事件后poller给channel注册的回调函数
*/

class EventLoop;
class Socket;
class Channel;



class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop *loop, const std::string &name,
                int sockfd,
                const InetAddr &localAddr,
                const InetAddr &perrAddr);
    ~TcpConnection();

    void connectionDestoryed();
    void connectionEstablished();

    void send(const std::string& buf);         /*发送数据*/
    void shutDown();                           /*关闭连接*/

    bool connected() { return state_ == kConnected; }
    EventLoop *getLoop() { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddr& localAddr() const { return localAddr_; }
    const InetAddr& peerAddr() const { return peerAddr_; }


    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = std::move(cb); }
    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = std::move(cb); }
    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = std::move(cb); }

private:
    /*给channel的相应的回调函数*/
    void handleRead(Timestamp receviTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message, size_t len);
    void shutDownInLoop();
private:

    enum StateE 
    {
        kDisConnected,   /*关闭状态*/
        kConnected,     /*连接状态*/
        kDisConnecting, /*正在断开*/
        kConnecting,    /*正在连接*/
    };
    void setState(StateE state) { state_ = state; }
    std::atomic_int state_;
    EventLoop *loop_;
    const std::string name_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    InetAddr localAddr_;
    InetAddr peerAddr_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    CloseCallback closeCallback_;
};

#endif