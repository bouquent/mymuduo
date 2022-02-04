#include <stdlib.h>
#include "logging.hpp"
#include "poller.hpp"
#include "epollpoller.hpp"
#include "pollpoller.hpp"

//在这里生成主要是为了让poller文件里面不要包含epollpoller和pollpoller文件
Poller* Poller::newDefaultPoller(EventLoop *loop, Poller::POLLER_OPT opt)
{
    if (opt == POLL_POLLER) {
        return new PollPoller(loop);
    } else if (opt == EPOLL_POLLER){
        return new EpollPoller(loop);
    } else {
        LOG_FATAL("[%s]:%s the opt is not epoll or poll!", __FILE__, __func__);
    }
}
