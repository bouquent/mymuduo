#include <iostream>
#include <string>
#include <functional>

#include <mymuduo/tcpserver.hpp>
#include <mymuduo/eventloop.hpp>

using namespace std::placeholders;


class Server
{
public:
    Server(EventLoop *loop, const InetAddr& addr, const std::string& argName)
        : server_(loop, addr, argName)
    {
        server_.setConnectionCallback(std::bind(&Server::onConnection, this, _1));
        server_.setMessageCallback(std::bind(&Server::onMessage, this, _1, _2, _3));

        server_.setThreadNum(1);
    }
    ~Server()
    {}

    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);

    void start() 
    {
        server_.start();
    }
private:
    TcpServer server_;
};

void Server::onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected()) {
        std::cout << "new Connection online" << conn->peerAddr().toIpPort() << std::endl;
    } else {
            std::cout << "new Connection offline" << conn->peerAddr().toIpPort() << std::endl;
    }
}


void Server::onMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
{
    std::string buf = buffer->retrieveAllAsString();                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
    conn->send(buf);
}


int main()
{
    EventLoop loop;
    InetAddr localAddr("127.0.0.1", 8888);
    Server server(&loop, localAddr, "czz");

    server.start();
    loop.loop();  /*主loop进入循环*/
    return 0;
}