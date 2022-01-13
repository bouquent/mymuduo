#include <stdlib.h>
#include "poller.hpp"
#include "epollpoller.hpp"
//#include "pollpoller.hpp"

//在这里生成主要是为了让poller文件里面不要包含epollpoller和pollpoller文件
Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if (getenv("USE pollPoller")) {
        return nullptr;
    } else {
        return new EpollPoller(loop);
    }
}
