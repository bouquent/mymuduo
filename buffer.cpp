#include "buffer.hpp"
   
#include <algorithm>
#include <unistd.h>
#include <sys/uio.h>
   
/*读取fd文件描述符中的数据*/
size_t Buffer::readFd(int sockfd, int *saveerrno)
{
    //因为tcp报文最大也只有65535
    char extrabuf[65536] = {0};

    /*防止剩余缓冲区无法接受所来的全部数据，所有使用一个临时的extrabuf帮助临时存储数据*/
    struct iovec vec[2];
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writeableBytes();
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    const int iovcnt = (writeableBytes() < sizeof(extrabuf)) ? 2 : 1; 

    ssize_t n = ::readv(sockfd, vec, iovcnt);
    if (n < 0) {
        *saveerrno = errno;
    } else if (n <= writeableBytes()) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writeableBytes());
    }

    return n;
}

/*向fd文件描述符写数据*/
size_t Buffer::writeFd(int sockfd, int *saveerrno)
{
    ssize_t n = ::write(sockfd, peek(), readableBytes());
    if (n < 0) {
        *saveerrno = errno;
    }
    return n;
}

void Buffer::retrieve(size_t len)                  /*将len长度的缓冲弄收回*/
{
    if (len < readableBytes()) {
        readerIndex_ += len;    /*回收一部分可读缓冲区*/
    } else {
        retrieveAll();
    }
}

void Buffer::retrieveAll()                         /*将所有可读缓冲区回收*/
{
    readerIndex_  = writerIndex_ = kCheapPrepend;
}

/*将所有可读数据转成string*/
std::string Buffer::retrieveAllAsString()          
{
    return retrieveAsString(readableBytes());
}

/*将长度为len的可读数据转成string*/
std::string Buffer::retrieveAsString(size_t len)  
{
    std::string result(peek(), len);
    retrieve(len);
    return result;
}



/*请保证可以写入len长度的数据(不够则扩容)*/
void Buffer::ensureWriteableBytes(size_t len)     
{
    if (len > writeableBytes()) {
        makeSpace(len);
    }
}

void Buffer::makeSpace(size_t len)
{
    if (readerIndex_ + writeableBytes() < len + kCheapPrepend) {
        buffer_.resize(writerIndex_ + len);
    } else {
        // | prepend | readerindex  | writeindex | 将读缓冲区前方的空余部分和写缓冲区之和大于所需写的大小，所以讲他们合并   
        size_t readable = readableBytes();
        // 搬运数据
        std::copy(begin() + readerIndex_
                , begin() + writerIndex_
                , begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend + readable;
    }
}

void Buffer::append(const char *data, size_t len)
{
    ensureWriteableBytes(len);
    std::copy(data, data + len, begin() + writerIndex_);
    writerIndex_ += len;
}