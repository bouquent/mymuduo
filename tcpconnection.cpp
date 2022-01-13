#include "tcpconnection.hpp"
#include "eventloop.hpp"
#include "channel.hpp"
#include "logging.hpp"
#include "callbacks.hpp"
#include "buffer.hpp"

#include <functional>
#include <errno.h>
#include <unistd.h>

using namespace std::placeholders;

static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if (nullptr == loop) {
        LOG_FATAL("tcpconnection loop cannot be nullptr!\n");
    }
    return loop;
}


TcpConnection::TcpConnection(EventLoop *loop, 
                const std::string &name,
                int sockfd,
                const InetAddr &localAddr,
                const InetAddr &perrAddr)
    : state_(kDisConnecting) 
    , loop_(CheckLoopNotNull(loop))
    , name_(name)
    , socket_(new Socket(sockfd))
    , channel_(new Channel(loop, sockfd))
    , localAddr_(localAddr)
    , peerAddr_(perrAddr)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_INFO("TcpConnection[%s], at fd = %d", name_.c_str(), channel_->fd());
    socket_->setKeepAlive(true);
}
    
TcpConnection::~TcpConnection()
{
    LOG_INFO("TcpConnection[%s], at fd = %d", name_.c_str(), channel_->fd());
}


/*连接建立*/
void TcpConnection::connectionEstablished()
{
    setState(kConnected);
    channel_->set_tie(shared_from_this());   /*将TcpConnection注册到channel中*/
    channel_->enableReading();               /*向poller中注册EPOLLIN事件*/

    connectionCallback_(shared_from_this()); /*客户设置的连接事件回调*/
}
/*连接销毁*/
void TcpConnection::connectionDestoryed()
{
    if (state_ == kConnected) {
        setState(kDisConnected);
        channel_->disableAll();         /*取消所有事件监听*/

        connectionCallback_(shared_from_this());  /*客户设置的连接事件回调*/
    }
    channel_->remove();                 /*从poller中摘除该channel_*/
}



/*发送数据*/
void TcpConnection::send(const std::string& buf)           
{
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            /*在执行这个函数的时候一般都是在执行MessageCallback，所以一般是在自己的线程中*/
             sendInLoop(buf.c_str(), buf.size());
        } else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }    
    }
}


void TcpConnection::sendInLoop(const void* message, size_t len)
{
    size_t nwrote = 0;
    size_t remaining = len;   /*剩余要写的长度*/
    bool faultError = false;

    if (state_ == kDisConnected || state_ == kDisConnecting) {
        LOG_ERROR("[%s]:%s don't send data when state is disconnect", __FILE__, __func__);
        return ;
    }
    
    /*没有可写事件，而且没有可读数据*/
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), message, len);

        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                /*数据发送完了，调用写完数据的回调函数*/
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this())
                    );
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                /*如果不是非阻塞写空返回的错误信号*/
                LOG_ERROR("[%s]:%s write error", __FILE__, __func__);
                if (errno == EPIPE || errno ==  ECONNRESET) {
                    faultError = true;
                }
            }
        }

        /*如果没有发送完所有的数据，并且write没有发生错误，说明数据太大
        * 注册EPOLLOUT事件来帮助发送数据，回调函数就是hanlewrite
        */
        if (!faultError && remaining > 0) {

            /*添加剩余数据*/
            outputBuffer_.append(static_cast<const char*>(message) + nwrote, remaining);
            if (!channel_->isWriting()) {
                channel_->enableWriting();
            }
        }
    }
}


/*关闭连接(单方面关闭)*/
void TcpConnection::shutDown()
{
    if (state_ == kConnected) {
        setState(kDisConnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutDownInLoop, this));
    }
}
void TcpConnection::shutDownInLoop()
{
    /*这里有两种情况
    * 1 有读写事件正在读，先暂时不关闭，等待它读完后，会再一次调用这个函数(根据kDisConnecting状态)进行关闭
    * 2 没有读写事件，直接调用关闭函数
    */
    if (!channel_->isWriting()) {
        socket_->shutDownWrite();
    }
}


/*读事件回调函数*/
void TcpConnection::handleRead(Timestamp receviTime)
{
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if (n < 0) {
        if (errno != EINTR) {
            errno = saveErrno;
            handleError();
        }
    } else if (n == 0) {
        /*客户端发送了fin报文,请求关闭*/
        handleClose();
    } else {
        messageCallback_(shared_from_this(), &inputBuffer_, receviTime);
    }

}

/*写事件回调函数*/
void TcpConnection::handleWrite()
{
    if (channel_->isWriting()) {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                /*缓冲区剩余数据为0,代表发生完成了,取消channel的写事件*/
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(&TcpConnection::writeCompleteCallback_, shared_from_this()));
                }
            }
            if (state_ == kDisConnecting) {
               //写数据的时候这个TcpConnection关闭了，在handlewrite里面一定是subloop在操作，所以直接调用shutdowninloop
                shutDownInLoop();
            }

        } else {
            LOG_ERROR("[%s]:%s write wrong, errno is %d", __FILE__, __func__, saveErrno);
        }

    }
}

//关闭回调函数
void TcpConnection::handleClose()
{
    setState(kDisConnected);
    channel_->disableAll();
    //channel_->remove()在closeCallback的connectionDestoryed中进行了调用

    connectionCallback_(shared_from_this());
    closeCallback_(shared_from_this());
}

//错误回调函数
void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof(optval);
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        LOG_ERROR("[%s]:%s getsocketopt wrong, errno is %d", __FILE__, __func__, errno);
    } else {
        LOG_ERROR("the sockfd error is %d", errno);
    }
}
