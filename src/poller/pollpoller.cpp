#include "pollpoller.hpp"
#include "logging.hpp"
#include "channel.hpp"
#include "eventloop.hpp"

#include <assert.h>


PollPoller::PollPoller(EventLoop *loop) 
    : Poller(loop)
{}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannel)
{
    int numEvent = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    if (numEvent > 0) {
        fillActiveListChannel(numEvent, activeChannel);
    } else if (numEvent == 0) {
        LOG_INFO("[%s]:%s nothing but poll is timeout!", __FILE__, __func__);
    } else {
        LOG_ERROR("[%s]:%s poller poll is error!", __FILE__, __func__);
    }


    Timestamp now = Timestamp::now();
    return now;
}

void PollPoller::fillActiveListChannel(int numEvent, ChannelList* activeChannel)
{
    for (int i = 0; i < pollfds_.size() && numEvent > 0; ++i) {
        if (pollfds_[i].revents > 0) {
            --numEvent;
            int sockfd = pollfds_[i].fd;
            auto iter = channels_.find(sockfd);
            assert(iter != channels_.end());

            Channel* channel = channels_[sockfd];
            channel->set_revents(pollfds_[i].revents);
            activeChannel->push_back(channel);
        }
    }
}

void PollPoller::updateChannel(Channel* channel) 
{
    if (channel->index() < 0) {
        //加入到poll中
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = channel->events();
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        channel->set_index(pollfds_.size() - 1);
        channels_.insert({channel->fd(), channel});
    } else {
        //更新channel在poll中的状态
        int idx = channel->index();
        assert(idx < pollfds_.size());

        pollfds_[idx].fd = channel->fd();
        pollfds_[idx].events = channel->events();
        pollfds_[idx].revents = 0;
        if (channel->isNoneEvent()) {
            pollfds_[idx].fd = -channel->fd() -1;  //设置为负值就是取消它的消息注册
        }
    }
}

void PollPoller::removeChannel(Channel* channel) 
{
    int idx = channel->index();
    assert(idx < pollfds_.size());
    struct pollfd pfd = pollfds_[idx];

    channels_.erase(channel->fd()); //从poller中摘除
    if (pollfds_.size() - 1 == static_cast<size_t>(idx)) {
        pollfds_.pop_back();
    } else {
        int sockfd = pollfds_.back().fd;  //获取最后一个元素的pollfd，以便之后修改它的下标
        if (sockfd < 0) {
            //如果小于0，说明它没有注册再poll上，但是仍然再poller中。
            sockfd = -sockfd - 1;
        }

        std::iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);     //将要删除的pollfd放到尾部，然后执行pop_back。        
        channels_[sockfd]->set_index(idx);
        pollfds_.pop_back();
    }
}