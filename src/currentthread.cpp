#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>

namespace CurrentThread
{
    __thread int t_cachedTid = 0;
    
    void cached()
    {
        if (0 == t_cachedTid) {
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}
