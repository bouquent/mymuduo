#include "poller.hpp"
#include "channel.hpp"

Poller::Poller(EventLoop* loop)
    : loop_(loop)
{}

Poller::~Poller()
{}

bool Poller::hasChannel(Channel* channel)
{
    auto iter = channels_.find(channel->fd());
    if (iter != channels_.end() && iter->second == channel) {
        return true;
    }
    return false;
}