#include "poller.hpp"

Poller::Poller(EventLoop *loop)
    : loop_(loop)
{}


bool Poller::hasChannel(Channel *channel)
{
    auto iter = channelMap_.find(channel->fd());
    if (iter != channelMap_.end() && iter->first == channel->fd()) 
        return true;
    return false;
}


Poller::~Poller() = default;
