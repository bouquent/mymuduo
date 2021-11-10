#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H

namespace CurrentThread
{
    extern __thread int t_cachedTid;
    void cacheTid();

    inline int tid() 
    {
        if (t_cachedTid == 0) {
            cacheTid();
        }
        return t_cachedTid;
    }
}


#endif