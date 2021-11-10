#ifndef SOCKET_H
#define SOCKET_H

#include "inetaddr.hpp"

class Socket
{
public:
    explicit Socket(int sockfd);
    ~Socket();

    void bindAddress(const InetAddr &LocalAddr);
    void listen();
    int accept(InetAddr *perrAddr);

    void shunDownWrite(bool on = true);
    //socketopt的一些设置
    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

    const int fd() const { return sockfd_; }
private:
    const int sockfd_;
};

#endif