#include "tcpserver.hpp"
#include "tcpconnection.hpp"
#include "inetaddr.hpp"
#include "logging.hpp"
#include "eventloop.hpp"

#include <memory>
#include <sys/socket.h>
#include <strings.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std::placeholders;

static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if (nullptr == loop) {
        LOG_FATAL("mainloop cannot be nullptr!\n");
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, 
                    const InetAddr& addr,
                    const std::string& nameArg,
                    Option option)
    : baseLoop_(CheckLoopNotNull(loop))
    , ipPort_(addr.toIpPort())
    , name_(nameArg)
    , acceptor_(new Acceptor(loop, addr))
    , threadPool_(new EventLoopThreadPool(loop, nameArg))
    , started_(0)
    , nextConnId_(0)
{
    /*用户连接的回调函数*/
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    for (auto iter : connections_) {
        auto conn = iter.second;
        iter.second.reset();    /*这里释放了资源后，conn出作用域也会自动释放资源*/

        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectionDestoryed, conn));
    }
}


/*启动服务*/
void TcpServer::start()
{
    if (started_++ == 0) {
        threadPool_->start(threadInitCallback_);

        /*开启监听，将acceptor加入mainloop中*/
        baseLoop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::setThreadNum(int threadNums)
{ 
    threadPool_->setThreadNum(threadNums); 
} 


/*接受到新用户后由acceptor调用这个回调，sockfd是接受到的用户socket文件描述符*/
void TcpServer::newConnection(int sockfd, const InetAddr &peerAddr)
{
    EventLoop *nextLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%s#%d", ipPort_.c_str(), nextConnId_++);
    std::string connName = name_ + buf;

    //获取客户端绑定的本机ip和端口号
    sockaddr_in local;
    ::bzero(&local, sizeof(local));
    socklen_t len = sizeof(local);
    if (::getsockname(sockfd, (struct sockaddr*) &local, &len) < 0) {
        LOG_ERROR("[%s]:%s getsockname error", __FILE__, __func__);
    }
    InetAddr localAddr(local);

    TcpConnectionPtr conn(new TcpConnection(nextLoop
                                        , connName
                                        , sockfd
                                        , localAddr
                                        , peerAddr
                                        ));
    connections_[connName] = conn;

    /*用户设置的回调函数*/
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    /*关闭连接的回调函数,这里的占位参数就是TcpConnectionPtr*/
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    
    //让该用户加入poller中进行监听                                    
    nextLoop->runInLoop(std::bind(&TcpConnection::connectionEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    baseLoop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    LOG_INFO("removeConnectionInLoop [%s] - connection %s", name_.c_str(), conn->name().c_str());
    connections_.erase(conn->name());

    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectionDestoryed, conn));
}
