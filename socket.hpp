#ifndef SOCKET_H
#define SOCKET_H

#include "noncopyable.hpp"
#include "inetaddr.hpp"


#include "inetaddr.hpp"

class Socket
{
public:
    explicit Socket(int sockfd);
    ~Socket();

    void bindAddr(const InetAddr &LocalAddr);
    void listen();
    int accept(InetAddr *perrAddr);

    void shutDownWrite(bool on = true);
    //socketopt的一些设置
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

    int fd() const { return sockfd_; }
private:
    const int sockfd_;
};

#endif