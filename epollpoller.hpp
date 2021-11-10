#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H
#include <vector>
#include "poller.hpp"
class Channel;


class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    /*timeoutMs是epoll_wait超时事件, activechannels是epoll_wait之后监听到的发生了事件的channel*/
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChanels) override;

    /*修改Poller中的channel的事件类型*/
    virtual void updateChannel(Channel* channel) override;
    /*将channel从poller中摘除*/
    virtual void removeChannel(Channel* channel) override;

private:
    /*更新channel在epoll中的注册事件*/
    void update(int operation, Channel* channel);
    void fillActiveChannels(int numEvents, ChannelList* channel);

private:
    int epollfd_;  /*epoll根节点*/

    static const int kInitEventListSize = 16;  /*events_初始长度*/
    std::vector<struct epoll_event> events_;   /*epoll_wait返回事件的容器*/
};


#endif