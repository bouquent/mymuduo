#ifndef POLLPOLLER_H
#define POLLPOLLER_H

#include "poller.hpp"
#include <sys/poll.h>
#include <vector>

class EventLoop;
class Channel;

class PollPoller : public Poller
{
public:
    using ChannelList = std::vector<Channel*>;

    PollPoller(EventLoop *loop);
    ~PollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activeChannel) override;

    void updateChannel(Channel *channel) override;
    void removeChannel(Channel* channel) override;
public:
    void fillActiveListChannel(int numEvent, ChannelList* activeChannel);

private:
    std::vector<struct pollfd> pollfds_;
};

#endif 