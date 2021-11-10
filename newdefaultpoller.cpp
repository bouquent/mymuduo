#include <stdlib.h>
#include "poller.hpp"
#include "epollpoller.hpp"
//#include "pollpoller.hpp"


Poller* Poller::newDefaultPoller(EventLoop *loop)
{
    if (getenv("USE pollPoller")) {
        return nullptr;
    } else {
        return new EpollPoller(loop);
    }
}
