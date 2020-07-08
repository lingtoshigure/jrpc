#pragma once

#include <string>
#include <string_view>

#include <cppJson/Value.h>

#include <libnet/EventLoop.h>
#include <libnet/TcpConnection.h>
#include <libnet/TcpServer.h>
#include <libnet/TcpClient.h>
#include <libnet/InetAddress.h>
#include <libnet/Buffer.h>
#include <libnet/Logger.h>
#include <libnet/Callbacks.h>
#include <libnet/Timestamp.h>
#include <libnet/ThreadPool.h>
#include <libnet/CountDownLatch.h>

namespace jrpc
{

using namespace std::literals::string_view_literals;
using namespace std::literals::chrono_literals;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

using net::EventLoop;
using net::TcpConnection;
using net::TcpServer;
using net::TcpClient;
using net::InetAddress;
using net::TcpConnectionPtr;
using net::noncopyable;
using net::Buffer;
using net::ConnectionCallback;
using net::ThreadPool;
using net::CountDownLatch;

using RpcDoneCallback = std::function<void(json::Value response)>; 

class UserDoneCallback
{
public:
    UserDoneCallback(json::Value &request, const RpcDoneCallback &callback)
    : request_(request),
      callback_(callback)
    { }

    void operator()(json::Value &&result) const
    {
        json::Value response(json::TYPE_OBJECT);
        response.addMember("jsonrpc", "2.0");
        response.addMember("id", request_["id"]);
        response.addMember("result", result);
        // 这个callback_ 才是最后的 回应客户端
        callback_(response);
    }

private:
    mutable json::Value request_;
    RpcDoneCallback callback_;
};

}
