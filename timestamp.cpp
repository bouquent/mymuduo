#include "timestamp.hpp"
#include <time.h>
#include <sys/time.h>
#include <iostream>

Timestamp::Timestamp(uint64_t mircoseconds)
    : mircoseconds_(mircoseconds) 
{}

Timestamp::~Timestamp() = default;

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t sec = tv.tv_sec * kmircoSecondsPreSecond;
    return Timestamp(sec + tv.tv_usec);
}

std::string Timestamp::toString() const
{
    time_t sec = mircoseconds_ / kmircoSecondsPreSecond;
    int usec = mircoseconds_ % kmircoSecondsPreSecond;
    struct tm* tm_time = localtime((time_t*) &sec);
    char buf[1024] = {0};
    snprintf(buf, 1024, "%4d/%02d/%02d %02d:%02d:%02d.%d"
            , tm_time->tm_year + 1900
            , tm_time->tm_mon + 1
            , tm_time->tm_mday
            , tm_time->tm_hour
            , tm_time->tm_min
            , tm_time->tm_sec
            , usec
            );
    return buf;
}

