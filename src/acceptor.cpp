#include "acceptor.hpp"
#include "logging.hpp"

#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>


int createSocket()
{
    /*设置成非阻塞socket文件描述符*/
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd == -1) {
        LOG_FATAL("[%s]:%s socket wrong!", __FILE__, __func__);
    }
    return sockfd;
}


Acceptor::Acceptor(EventLoop *loop, const InetAddr& addr, bool option)
    : baseLoop_(loop)
    , acceptSocket_(createSocket())
    , acceptChannel_(loop, acceptSocket_.fd())   //这里注意acceptorSocket_要在acceptChannel_之前声明
    , listening_(false)
{
    acceptSocket_.setReuseAddr(true);       //设置一定要在bind之前
    acceptSocket_.setReusePort(option);
    acceptSocket_.setKeepAlive(true);

    acceptSocket_.bindAddr(addr);

    /*有新用户连接，则调用回调handleRead*/
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}
    
Acceptor::~Acceptor()
{
    /*取消所有事件*/
    acceptChannel_.disableAll();
    /*从poller中摘除*/
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();

    LOG_INFO("server started to listen!");
}

void Acceptor::handleRead()
{
    InetAddr peeraddr;
    int connfd = acceptSocket_.accept(&peeraddr);
    if (connfd > 0) {
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peeraddr);
        } else {
            LOG_ERROR("[%s]:%s newConnctionCallback is nullptr, cannot connect new client!", __FILE__, __func__);
            ::close(connfd);
        }

    } else {
        LOG_ERROR("[%s]:%s accept error, errno is %d", __FILE__, __func__, errno);
        if (errno == EMFILE) {
            LOG_ERROR("accpet error because the file open too much!");
        }
    }
}

