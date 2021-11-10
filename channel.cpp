 #include "channel.hpp"
 #include <sys/epoll.h>
 #include "eventloop.hpp"
 
 const int Channel::kNoneEvent = 0;
 const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
 const int Channel::kWriteEvent = EPOLLOUT;
 
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , tied_(false)
    , index_(-1)
{} 


Channel::~Channel()
{}


void Channel::handleEvent(Timestamp time)
{
  /*判断上层连接TcpConnection还存在，因为它的回调函数都是TcpConnection设置的*/
  if (tied_) {
    /*将弱指针升级为强指针*/
    std::shared_ptr<void> ptr = tie_.lock();
    if (ptr != nullptr) {
      handleEventWithGuard(time);
    }
  } 
  else {
    /*acceptorchannel没有绑定TcpConnection,使用这条路*/
    handleEventWithGuard(time);
  }
}

/*根据对应的事件执行相应的回调函数*/
void Channel::handleEventWithGuard(Timestamp time)
{
  if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
    if (closeCallback_) {
      closeCallback_();
    }
  }

  if (revents_ & (EPOLLPRI | EPOLLIN)) {
    if (readCallback_) 
       readCallback_(time);
  }

  if (revents_ & EPOLLERR) {
    if (errorCallback_)
       errorCallback_();
  }

  if (revents_ & EPOLLOUT) {
    if (writeCallback_) 
       writeCallback_();
  }
}


void Channel::set_tie(const std::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}



/*更新当前fd的events在poller中的类型*/
void Channel::update()
{
  loop_->updateChannel(this);
}
/*在对应的poller中删除这个channel*/
void Channel::remove()
{
  loop_->removeChannel(this);
}
