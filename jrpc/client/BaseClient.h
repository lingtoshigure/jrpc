#pragma once 

#include <unordered_map>

#include <cppJson/Value.h>
#include <jrpc/util.h>

namespace jrpc
{

using ResponseCallback = std::function<void(json::Value&, bool isError, bool isTimeout)>;

class BaseClient: noncopyable
{
public:
    BaseClient(EventLoop* loop, const InetAddress& serverAddress)
    : id_(0),
      client_(loop, serverAddress)
    {
        client_.setMessageCallback([this](const auto& connptr, auto& buffer)
                                    { 
                                        this->onMessage(connptr, buffer);
                                    });
    }

    void start() { client_.start(); }

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        client_.setConnectionCallback(cb);
    }

    void sendCall(const TcpConnectionPtr& conn, json::Value& call, const ResponseCallback& cb);

    void sendNotify(const TcpConnectionPtr& conn, json::Value& notify);

private:
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void handleMessage(Buffer& buffer);
    void handleResponse(std::string& json);
    void handleSingleResponse(json::Value& response);
    void validateResponse(json::Value& response);
    void sendRequest(const TcpConnectionPtr& conn, json::Value& request);

private:
    using Callback = std::unordered_map<int64_t, ResponseCallback>;

    int64_t   id_;
    Callback  callbacks_;
    TcpClient client_;
};


}
