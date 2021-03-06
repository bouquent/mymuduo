#ifndef INETADDR_H
#define INETADDR_H

#include "copyable.hpp"
#include <netinet/in.h>
#include <string>

class InetAddr : copyable
{
public:
    explicit InetAddr(const std::string& ip = "127.0.0.1", uint16_t port = 0);
    explicit InetAddr(const struct sockaddr_in& addr);

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    struct sockaddr_in getSockAddr() const { return addr_; }
    void setSockAddr(struct sockaddr_in addr) { addr_ = addr; }
private:
    struct sockaddr_in addr_;
};
#endif 
