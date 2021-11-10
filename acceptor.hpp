#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "channel.hpp"
#include "socket.hpp"
#include "inetaddr.hpp"

#include <functional>

class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int, const InetAddr&)>;

    Acceptor(EventLoop *loop, const InetAddr& addr);
    ~Acceptor();
    void setNewConnectionCallback(const NewConnectionCallback &cb) { newConnectionCallback_ = std::move(cb); }

    void listen();
    bool listening() const { return listening_; }

    Acceptor* get() { return this;}
private:
    void handleRead();  /*监听到新用户的回调*/

private:
    EventLoop *baseLoop_;  /*mainloop*/
    Socket acceptSocket_;
    Channel acceptChannel_;
    bool listening_;

    NewConnectionCallback newConnectionCallback_; /*监听到新用户的更上层回调(handleRead调用)*/
};

#endif