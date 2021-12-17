#include "socket.hpp"
#include "inetaddr.hpp"
#include "logging.hpp"

#include <strings.h>
#include <unistd.h>
#include <netinet/tcp.h>

Socket::Socket(int sockfd)
    : sockfd_(sockfd) 
{}

Socket::~Socket()
{
    ::close(sockfd_);
}


void Socket::bindAddress(const InetAddr &LocalAddr)
{
    struct sockaddr_in ser_addr = LocalAddr.getSockAddr();
    int ret = ::bind(sockfd_, (struct sockaddr*) &ser_addr, sizeof(ser_addr));
    if (ret != 0) {
        LOG_FATAL("[%s]:%s bind wrong!", __FILE__, __func__);
    }
}

void Socket::listen()
{
    int ret = ::listen(sockfd_, 1024);
    if (ret != 0) {
        LOG_FATAL("[%s]:%s listen wrong!", __FILE__, __func__);
    }
}

int Socket::accept(InetAddr *peerAddr)
{
    struct sockaddr_in cli_addr;
    bzero(&cli_addr, sizeof(cli_addr));
    socklen_t len = sizeof(cli_addr);
    int connfd = ::accept(sockfd_, (struct sockaddr*) &cli_addr, &len);
    if (connfd >= 0) {
        peerAddr->setSockAddr(cli_addr);
    }
    return connfd;
}


void Socket::shunDownWrite(bool on)
{
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_ERROR("[%s]:%s shundown error!\n", __FILE__, __func__);
    }
}

//socketopt的一些设置
void Socket::setTcpNoDelay(bool on)
{
    int opt = on ? 1 : 0;
    int ret = setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
    if (ret != 0) {
        LOG_ERROR("[%s]:%s setsockopt wrong!", __FILE__, __func__);
    }    
}

void Socket::setReuseAddr(bool on)
{
    int opt = on ? 1 : 0;
    int ret = setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (ret != 0) {
        LOG_ERROR("[%s]:%s setsockopt wrong!", __FILE__, __func__);
    }
}

void Socket::setReusePort(bool on)
{
    int opt = on ? 1 : 0;
    int ret = setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    if (ret != 0) {
        LOG_ERROR("[%s]:%s setsockopt wrong!", __FILE__, __func__);
    }
}

void Socket::setKeepAlive(bool on)
{
    int opt = on ? 1 : 0;
    int ret = setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
    if (ret != 0) {
        LOG_ERROR("[%s]:%s setsockopt wrong!", __FILE__, __func__);
    }
}
