#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H

#include "poller.hpp"
#include "timestamp.hpp"

#include <sys/epoll.h>
#include <vector>


class EventLoop;
class Channel;

class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    /*修改Poller中的channel的事件类型*/
    void updateChannel(Channel* channel) override;
    /*将channel从poller中摘除*/
    void removeChannel(Channel* channel) override;

    /*timeoutMs是epoll_wait超时事件, activechannels是epoll_wait之后监听到的发生了事件的channel*/
    Timestamp poll(int timeoutMs, ChannelList* activeChannel) override;

private:
    /*更新channel在epoll中的注册事件*/
    void update(int operation, Channel* channel);

    void fillActiveListChannel(int eventNums, ChannelList* activeChannel);

private:
    int epollfd_;

    std::vector<struct epoll_event> events_;   /*epoll_wait返回事件的容器*/
    static const int KinitEventListSize = 16;   /*events_初始长度*/
};

#endif