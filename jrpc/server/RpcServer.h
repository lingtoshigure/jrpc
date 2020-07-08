#pragma once

#include <memory>
#include <unordered_map>

#include <cppJson/Value.h>

#include <jrpc/util.h>
#include <jrpc/server/RpcService.h>
#include <jrpc/server/BaseServer.h>

namespace jrpc
{

/// @brief: 在一个RpcServer 中有多个 RpcService 
///         method.name <--> method 映射关系
///         
///         这个类本质上是 TcpServer 的一个子类，因此是用来监听和处理各个客户端的rpc请求，然后处理这些服务
///         比如一个 "Echo" 服务， "acmulate" 服务 等，一个 @c RpcServer 可以支持多个的服务
///          一个 RpcServer  --> 多个 RpcService
///          一个 RpcService --> 多个 Procedure
class RpcServer: public BaseServer<RpcServer>
{
public:
    RpcServer(EventLoop* loop, const InetAddress& listen)
    : BaseServer(loop, listen)
    {}

    ~RpcServer() = default;

    // used by user stub
    void addService(std::string_view serviceName, RpcService* service);

    // 真正用来处理请求的函数
    void handleRequest(const std::string& json, const RpcDoneCallback& done);

private:
    void handleSingleRequest(json::Value& request,  const RpcDoneCallback& done);
    void handleBatchRequests(json::Value& requests, const RpcDoneCallback& done);
    void handleSingleNotify(json::Value& request);

    void validateRequest(json::Value& request);
    void validateNotify(json::Value& request);
    
    std::unordered_map<std::string_view, std::unique_ptr<RpcService>> services_;
    
};

}
