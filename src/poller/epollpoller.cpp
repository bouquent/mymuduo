#include "epollpoller.hpp"
#include "logging.hpp"
#include "channel.hpp"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>


const int kNew = -1;    //从未添加到epollfd中
const int kAdded = 1;   //已添加到epollfd中
const int kDeleted = 2; //不在epollfd中监听，但是仍注册在poller中

EpollPoller::EpollPoller(EventLoop* loop)
        : Poller(loop)
        , epollfd_(::epoll_create(16))
        , events_(KinitEventListSize)
{}

EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}


Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannel)
{
    int eventNums = epoll_wait(epollfd_, &*events_.begin(), events_.size(), timeoutMs);
    int saveerrno = errno;/*记录调用epoll_wait后的错误状态*/ 
    
    if (eventNums > 0) {
        if (eventNums == events_.size()) {
             /*事件太多，将events_进行扩容*/
            events_.resize(events_.size() * 2);
        }
        LOG_INFO("%d event happened", eventNums);
        fillActiveListChannel(eventNums, activeChannel);
    } else if (eventNums == 0) {
        LOG_INFO("nothing happened but the epoll_wait timeout!");
    } else {
        if (errno != EINTR) {
            /*输出调用epoll_wait后的错误状态*/
            LOG_ERROR("[%s]:[%s] epol_wait error, errno is %d!", __FILE__, __func__, saveerrno);
        }
    }

    return Timestamp::now();
}

void EpollPoller::fillActiveListChannel(int eventNums, ChannelList* activeChannel)
{  
    for (int i = 0; i < eventNums; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannel->push_back(channel);
    }
}



void EpollPoller::updateChannel(Channel* channel)
{
    int index = channel->index();
    int fd = channel->fd();

    if (index == kNew || index == kDeleted) {
        if (index == kNew) {
            channels_.insert({fd, channel});
        }
        
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        if (channel->isNoneEvent()) {
            //这个channel没有任何事情监听了，可以从红黑树上取下来
            channel->set_index(kDeleted);

            update(EPOLL_CTL_DEL, channel);
        }
        else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}


void EpollPoller::removeChannel(Channel* channel) 
{
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);

    if (index == kAdded) {
        //在红黑树上摘除这个channel的监听事件
        update(EPOLL_CTL_DEL, channel);
    }

    channels_.erase(channel->fd());

    channel->set_index(kNew);
}


/*更新channel在epoll中的注册事件*/
void EpollPoller::update(int operation, Channel* channel)
{
    int fd = channel->fd();

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.data.ptr = channel;
    ev.events = channel->events();
    
    int ret = epoll_ctl(epollfd_, operation, fd, &ev);

    if (ret != 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_ERROR("[%s]:[%s] epoll_ctl_del error!", __FILE__, __func__);
        } else {
            LOG_FATAL("[%s]:[%s] epoll_ctl_mod error!", __FILE__, __func__);
        }
    }
}


