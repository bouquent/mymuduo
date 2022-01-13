#ifndef CURRENTTHREAD_H
#define CURRENTTHREAD_H


namespace CurrentThread
{
    extern __thread int t_cachedTid;
    void cached();
    inline int tid()
    {
        if (0 == t_cachedTid) {
            cached();
        }
        return t_cachedTid;
    }
}


#endif