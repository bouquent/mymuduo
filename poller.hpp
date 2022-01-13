#ifndef POLLER_H
#define POLLER_H

#include <vector>
#include <map>

#include "noncopyable.hpp"
#include "timestamp.hpp"

class Channel;
class EventLoop;

class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;
    Poller(EventLoop *loop = nullptr);
    virtual ~Poller();

    /*给所有的的io复用接口提供统一的Poll*/
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannel) = 0;

    /*修改Poller上的channel*/
    virtual void updateChannel(Channel*) = 0;
    virtual void removeChannel(Channel* ) = 0;

    /*当前Poller中是否有channel*/
    bool hasChannel(Channel*);

    /*生成一个对应的Poller实例(epoll或poll)*/
    static Poller* newDefaultPoller(EventLoop*);
protected:
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* loop_;
};


#endif 

