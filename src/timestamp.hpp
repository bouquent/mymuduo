#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include "copyable.hpp"

#include <string>

class Timestamp : public copyable
{
public:
    explicit Timestamp(uint64_t mircoseconds = 0);
    ~Timestamp();

    bool vaild() const { return mircoseconds_ > 0; }
    uint64_t mircoseconds() const { return mircoseconds_; }

    static Timestamp now();
    static Timestamp invaild() { return Timestamp(0); } /*生成一个被初始化为0的对象*/
    std::string toString() const;

    static const int kmircoSecondsPreSecond = 1000 * 1000;
private:
    uint64_t mircoseconds_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.mircoseconds() < rhs.mircoseconds();
}
inline bool operator>(Timestamp lhs, Timestamp rhs)
{
    return lhs.mircoseconds() > rhs.mircoseconds();
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    uint64_t delta = static_cast<int64_t>(seconds * Timestamp::kmircoSecondsPreSecond);
    return Timestamp(delta + timestamp.mircoseconds());
}

#endif 