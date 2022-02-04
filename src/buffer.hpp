#ifndef BUFFER_H
#define BUFFER_H

#include "copyable.hpp"

#include <vector>
#include <string>

using std::size_t;

/*  kCheapPrepend                   readerIndex_                    writerIndex_
/*        |           头部大小           |           可读缓冲区           |            可写缓冲区        
*/
class Buffer : copyable
{
public:
    /*记录缓冲区的长度*/
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
            : buffer_(kInitialSize)
            , readerIndex_(kCheapPrepend)
            , writerIndex_(kCheapPrepend)
        {}

    /*向fd中读写数据*/
    size_t readFd(int sockfd, int *saveErrno);
    size_t writeFd(int sockfd, int *saveErrno);

    void retrieve(size_t len);                  /*将len长度的缓冲弄收回*/
    void retrieveAll();                         /*将所有可读缓冲区回收*/

    std::string retrieveAllAsString();          /*将所有可读数据转成string*/
    std::string retrieveAsString(size_t len);   /*将长度为len的可读数据转成string*/

    void ensureWriteableBytes(size_t len);     /*请保证可以写入len长度的数据(不够则扩容)*/
    void makeSpace(size_t len);
    void append(const char *data, size_t len);  /*将数据添加到写缓冲区中*/

    /*缓冲区可读数据首地址*/
    const char* peek() const { return begin() + readerIndex_; } 
    /*首部长度*/
    size_t prependableBytes() const { return readerIndex_; }
    /*可读数据长度*/
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    /*可写缓冲区长度*/
    size_t writeableBytes() const { return buffer_.size() - writerIndex_; }

private:
    char* begin() { return &*buffer_.begin(); }
    const char* begin() const { return &*buffer_.begin(); }
    char* beginWrite() { return &*(buffer_.begin() + writerIndex_); }
    const char* beginWrite() const { return &*(buffer_.begin() + writerIndex_); }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

};

#endif