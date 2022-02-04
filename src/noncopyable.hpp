#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

class noncopyable
{
protected:
    noncopyable() = default;
    ~noncopyable() = default;
public:
    noncopyable(const noncopyable& n) = delete;
    noncopyable& operator= (const noncopyable& n) = delete;
};

#endif