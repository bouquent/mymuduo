#include "inetaddr.hpp"
#include <cstring>
#include <arpa/inet.h>
#include <strings.h>


InetAddr::InetAddr(const std::string& ip, uint16_t port)
{
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    addr_.sin_port = htons(port);
}

InetAddr::InetAddr(const struct sockaddr_in& addr) 
    : addr_(addr)
{}


std::string InetAddr::toIp() const
{
    char buf[128];
    inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddr::toIpPort() const
{
    char buf[128];
    inet_ntop(AF_INET, &addr_.sin_addr.s_addr, buf, sizeof(buf));
    
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + strlen(buf), "%d", port);
    return buf;  
}

uint16_t InetAddr::toPort() const
{
    uint16_t port = ntohs(addr_.sin_port);
    return port;
}
