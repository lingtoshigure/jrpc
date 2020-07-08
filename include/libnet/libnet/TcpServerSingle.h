#pragma once

#include <unordered_set>
#include <memory>

#include <libnet/Callbacks.h>
#include <libnet/Acceptor.h>

namespace net
{

class EventLoop;
class TcpServerSingle : noncopyable {
public:
  TcpServerSingle(EventLoop *loop, const InetAddress &local);

  void setConnectionCallback(const ConnectionCallback &cb)       { connectionCallback_    = cb; }
  void setMessageCallback(const MessageCallback &cb)             { messageCallback_       = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
  void start();
private:
  void newConnection(int connfd, const InetAddress &local, const InetAddress &peer);
  void closeConnection(const TcpConnectionPtr &conn);

  using ConnectionSet = std::unordered_set<TcpConnectionPtr> ;

  EventLoop*                 loop_;
  std::unique_ptr<Acceptor>  acceptor_;
  ConnectionSet              connections_;         // 一个服务器的所有连接对象
  ConnectionCallback         connectionCallback_;  // 新连接到来时候的回调函数
  MessageCallback            messageCallback_;     
  WriteCompleteCallback      writeCompleteCallback_;
};

}
