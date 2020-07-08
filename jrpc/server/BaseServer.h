#pragma once

#include <cppJson/Value.h>

#include <jrpc/RpcError.h>
#include <jrpc/util.h>

namespace jrpc
{

class RequestException;

/**
 * 让子类作基类的模板参数，即CRTP技术
 * 
 * 基类，就TcpServer的一个wrapper，设置了一些回调函数
*/
template <typename ProtocolServer>
class BaseServer: noncopyable
{
public:
    void setNumThread(size_t n) { server_.setNumThread(n); }

    void start() { server_.start(); }

protected:
    BaseServer(EventLoop* loop, const InetAddress& listen);
    ~BaseServer() = default;

private:
    // 这些都是读取函数
    void onConnection(const TcpConnectionPtr& conn);
    void onMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    void onHighWatermark(const TcpConnectionPtr& conn, size_t mark);
    void onWriteComplete(const TcpConnectionPtr& conn);

    void handleMessage(const TcpConnectionPtr& conn, Buffer& buffer);
    // 写函数
    void sendResponse(const TcpConnectionPtr& conn, const json::Value& response);

    // 基类转化为子类
    ProtocolServer& convert();
    const ProtocolServer& convert() const;

protected:
    json::Value wrapException(RequestException& e);

private:
    TcpServer server_;
};


}
