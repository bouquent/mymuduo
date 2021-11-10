#include "timestamp.hpp"
#include <time.h>
#include <iostream>

Timestamp::Timestamp(uint64_t mircoseconds)
    : mircoseconds_(mircoseconds) 
{}

Timestamp::~Timestamp() = default;

Timestamp Timestamp::now()
{
    time_t tm = time(NULL);
    return Timestamp(tm);
}

std::string Timestamp::toString() const
{
    struct tm* tm_time = localtime((time_t*) &mircoseconds_);
    char buf[1024];
    snprintf(buf, 1024, "%4d/%02d/%02d %02d:%02d:%02d"
            , tm_time->tm_year + 1900
            , tm_time->tm_mon + 1
            , tm_time->tm_mday
            , tm_time->tm_hour
            , tm_time->tm_min
            , tm_time->tm_sec
            );
    return buf;
}
