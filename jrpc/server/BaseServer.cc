#include <cppJson/Document.h>
#include <cppJson/Writer.h>
#include <cppJson/StringWriteStream.h>

#include <jrpc/Exception.h>
#include <jrpc/server/BaseServer.h>
#include <jrpc/server/RpcServer.h>

using namespace jrpc;

namespace
{

const size_t kHighWatermark = 65536;
const size_t kMaxMessageLen = 100 * 1024 * 1024; // 100M

}

namespace jrpc
{

// 目前只能传入 RpcServer 
template class BaseServer<RpcServer>; 

}

template <typename ProtocolServer>
BaseServer<ProtocolServer>::BaseServer(EventLoop *loop, const InetAddress& listen)
: server_(loop, listen)
{
    server_.setConnectionCallback([this](const auto& connptr){ this->onConnection(connptr); });
    server_.setMessageCallback([this](const auto& connptr, auto& buffer){ this->onMessage(connptr, buffer);});
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onConnection(const TcpConnectionPtr& connptr)
{
    if (connptr->connected()) 
    {
        DEBUG("connection %s is [up]", connptr->peer().toIpPort().c_str());
        connptr->setHighWaterMarkCallback([this](const auto& connp, size_t mark)
                                          { 
                                            this->onHighWatermark(connp, mark); 
                                          }, 
                                          kHighWatermark);
    }
    else 
    {
        DEBUG("connection %s is [down]", connptr->peer().toIpPort().c_str());
    }
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onMessage(const TcpConnectionPtr& connptr, Buffer& buffer)
{
    try 
    {
        handleMessage(connptr, buffer);
    }
    catch (RequestException& e) 
    {
        json::Value response = wrapException(e);
        sendResponse(connptr, response);
        connptr->shutdown();

        WARN("BaseServer::onMessage() %s request error: %s",
             connptr->peer().toIpPort().c_str(),
             e.what());
    }
    catch (NotifyException& e)
    {
        WARN("BaseServer::onMessage() %s notify error: %s",
             connptr->peer().toIpPort().c_str(), 
             e.what());
    }
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onHighWatermark(const TcpConnectionPtr& connptr, size_t mark)
{
    DEBUG("connection %s high watermark %lu",
          connptr->peer().toIpPort().c_str(), 
          mark);

    connptr->setWriteCompleteCallback([this](const auto& connp){ this->onWriteComplete(connp); });
    connptr->stopRead();
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::onWriteComplete(const TcpConnectionPtr& connptr)
{
    DEBUG("connection %s write complete",
         connptr->peer().toIpPort().c_str());
    connptr->startRead();
}

/** @brief: 这是一个Rpc服务，因此在处理可读事件需要处理的是和rpc有关的任务
 *           就是解析 buffer，里面的数据是以 json的数据格式，因此要以 json 格式解析
 *  @param: @c buffer 存储的是此次请求的数据，其格式是 : 数据长度 + crlf + 数据 + clrf
 *  
 *  @core: 最终用来处理此次请求的是子类的 @c handleRequest 函数
*/
template <typename ProtocolServer>
void BaseServer<ProtocolServer>::handleMessage(const TcpConnectionPtr& connptr, Buffer& buffer)
{
    while (true) {

        const char *crlf = buffer.findCRLF();
        // 什么也没有
        if (crlf == nullptr)
            break;
        // 空行
        if (crlf == buffer.peek()) {
            buffer.retrieve(2);
            break;
        }

        // 报头长度
        size_t headerLen = crlf - buffer.peek() + 2;

        json::Document header;
        auto err = header.parse(buffer.peek(), headerLen);
        if (err != json::PARSE_OK ||
            !header.isInt32() ||
            header.getInt32() <= 0)
        {
            throw RequestException(RPC_INVALID_REQUEST, "invalid message length");
        }

        auto jsonLen = static_cast<uint32_t>(header.getInt32());
        if (jsonLen >= kMaxMessageLen)
            throw RequestException(RPC_INVALID_REQUEST, "message is too long");

        if (buffer.readableBytes() < headerLen + jsonLen)
            break;

        buffer.retrieve(headerLen);
        // 消息体
        auto json = buffer.retrieveAsString(jsonLen);
        /// @brief: RpcServer::handleRequest(const std::string& json, const RpcDoneCallback& done)
        /// @param: 第二个参数 lambda 表达式类型是 @c RpcDoneCallback，等处理完此次客户端的请求，再调用的
        ///          将此次结果，返回给客户端。
        //           格式： 此次数据包的总长度 + clrf + 内容 + clrf
        convert().handleRequest(json, 
                                [connptr, this](json::Value response) 
                                {
                                    if (!response.isNull()) 
                                    {
                                        // 等处理完毕，再发送回应客户端的函数
                                        sendResponse(connptr, response);
                                        TRACE("BaseServer::handleMessage() %s request success",
                                              connptr->peer().toIpPort().c_str());
                                    }
                                    else {
                                        TRACE("BaseServer::handleMessage() %s notify success",
                                              connptr->peer().toIpPort().c_str());
                                    }
                                });
    }
}

template <typename ProtocolServer>
json::Value BaseServer<ProtocolServer>::wrapException(RequestException& e)
{
    json::Value response(json::TYPE_OBJECT);
    response.addMember("jsonrpc", "2.0");
    auto& value = response.addMember("error", json::TYPE_OBJECT);
    value.addMember("code", e.err().asCode());
    value.addMember("message", e.err().asString());
    value.addMember("data", e.detail());
    response.addMember("id", e.id());
    return response;
}

template <typename ProtocolServer>
void BaseServer<ProtocolServer>::sendResponse(const TcpConnectionPtr& connptr, const json::Value& response)
{
    json::StringWriteStream os;
    json::Writer writer(os);
    response.writeTo(writer);

    // 长度:内容长度 + clrf 两个字节长度
    // 内容
    auto message = std::to_string(os.get().length() + 2)
                                 .append("\r\n")
                                 .append(os.get())
                                 .append("\r\n");
    connptr->send(message);
}

template <typename ProtocolServer>
ProtocolServer& BaseServer<ProtocolServer>::convert()
{
    // 利用crtp技术, 实现静态多态
    return static_cast<ProtocolServer&>(*this);
}

template <typename ProtocolServer>
const ProtocolServer& BaseServer<ProtocolServer>::convert() const
{
    return static_cast<const ProtocolServer&>(*this);
}


