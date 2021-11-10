#ifndef CHANNEL_H
#define CHANNEL_H
#include "timestamp.hpp"
#include "noncopyable.hpp"
#include <functional>
#include <memory>

class EventLoop;

class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();

    /*用于处理事件的函数*/
    void handleEvent(Timestamp time);


    int fd() const { return fd_; }
    int events() const { return events_; }
    int revents() const { return revents_; }
    int index() const { return index_;}
    EventLoop* onwer_loop() const {return loop_;}
    void set_revents(int revents) { revents_ = revents; }   /*被poller使用，poller监听到事件发生，通过这个函数来设置事件已经发生*/
    void set_index(int index) { index_ = index; }
    void set_tie(const std::shared_ptr<void>& obj);

    /*设置fd的事件类型*/
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &- ~kWriteEvent; update(); }
    void disableAll() { events_ &= 0; update(); }

    //当前events的状态
    bool isReading() { return events_ &= kReadEvent; }
    bool isWriting() { return events_ &= kWriteEvent; }
    bool isNoneEvent() { return events_ == kNoneEvent; }


    /*设置相应的回调函数*/
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }

    /*在对应的poller中删除这个channel*/
    void remove();
private:
    /*更新当前fd的events在poller中的类型*/
    void update();
    /*根据对应的事件执行相应的回调函数*/
    void handleEventWithGuard(Timestamp time);


private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;   /*fd对应的subloop,作为channel和poller的中间件*/
    const int fd_;      /*客户的socket文件描述符*/
    int events_;        /*给fd上注册的事件类型*/
    int revents_;       /*fd上具体发生的事件类型*/
    int index_;         /*Channel是否添加到了Poller中*/

    std::weak_ptr<void> tie_;  /*保证TcpConnection的存活*/
    bool tied_;             /*tie_是否绑定过shared_ptr*/

    /*只有channel通过revents才能知道对应的fd上发生了什么事件，所以由channel来调用相应的回调函数*/
    EventCallback closeCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    ReadEventCallback readCallback_;
};

#endif