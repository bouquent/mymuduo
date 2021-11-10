#include "epollpoller.hpp"
#include "channel.hpp"
#include "logging.hpp"
#include <sys/epoll.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

const int kNew = -1;        /*Channel未添加到poller中*/
const int kAdded = 1;       /*Channel已添加到poller中*/
const int kDeleted = 2;     /*Channel已从poller中删除*/

EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop)
    , epollfd_(::epoll_create(16)) 
    , events_(kInitEventListSize)
{
    if (epollfd_ == -1) {
        LOG_FATAL("[%s]:%s epoll_create error!", __FILE__, __func__);
    }
}

EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}

Timestamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    LOG_INFO("[%s]:%s fd total count is %ld",__FILE__, __func__, channelMap_.size());

    int numEvents = epoll_wait(epollfd_, &(*events_.begin()), events_.size(), timeoutMs);
    int saveErrno = errno; /*记录调用epoll_wait后的错误状态*/ 

    if (numEvents > 0) {
        LOG_INFO("%d events happened!", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (events_.size() == numEvents) {
            /*事件太多，将events_进行扩容*/
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        /*没有任何事件发生，只是单纯超时了*/
        LOG_INFO("epoll_wait timeout but onthing happened!");
    } else {
        if (errno != EINTR) {
            /*输出调用epoll_wait后的错误状态*/
            LOG_ERROR("[%s]:%s epoll_wait error, errno is %d", __FILE__, __func__, saveErrno);
        }
    }
    return Timestamp::now();
}

void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels)
{
    for (int i = 0; i < numEvents; ++i) {
        Channel *cal = static_cast<Channel*>(events_[i].data.ptr);
        cal->set_revents(events_[i].events);
        activeChannels->push_back(cal);
    }
}

/*修改Poller中的channel*/
void EpollPoller::updateChannel(Channel* channel) 
{
    int fd = channel->fd();
    int index = channel->index();
    if (index == kNew || index == kDeleted) {
        /*index如果不是kAdded，修改事件只可能是添加事件*/
        if (kNew == index) 
        {
            channelMap_.insert({fd, channel});  /*将新的fd加入poller的channelMap集合中*/
        }
        else
        {
            /*已经在poller中注册过(在ChannelMap中存在)，但是处于未监听状态(在epollfd中已经被删除)*/
            if (channelMap_.find(fd) == channelMap_.end()) {
                LOG_FATAL("[%s]:%s, updatechannel kDeleted's channel is not placed in channelMap", __FILE__, __func__);
            }
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        if (channel->isNoneEvent()) 
        {
            /*channel中没有已经任何事件了,将它从epollfd上取出*/
            channel->set_index(kDeleted);
            update(EPOLL_CTL_DEL, channel);
        }
        else 
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

/*将channel从poller中摘除*/
void EpollPoller::removeChannel(Channel* channel)
{
    int fd = channel->fd();
    int index = channel->index();
    assert(index == kNew || index == kDeleted); /*channel在poller中才能删除*/

    /*从poller中摘除*/
    channelMap_.erase(fd);

    if (index == kAdded) {
        /*从epollfd中摘除*/
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

/*更新channel在epoll中的注册事件*/
void EpollPoller::update(int operation, Channel* channel)
{
    int fd = channel->fd();

    struct epoll_event ep;
    memset(&ep, 0, sizeof(ep));
    ep.data.fd = channel->fd();
    ep.data.ptr = channel;
    ep.events = channel->events();

    int ret = epoll_ctl(epollfd_, operation, fd, &ep);
    if (ret < 0) {
        if (operation == EPOLL_CTL_DEL)
            LOG_ERROR("[%s]:%s epoll_ctl error!", __FILE__, __func__);
        else 
            LOG_FATAL("[%s]:%s epoll_ctl error!", __FILE__, __func__);
    }
}