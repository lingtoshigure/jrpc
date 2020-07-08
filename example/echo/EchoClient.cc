#include <iostream>

#include "example/echo/EchoClientStub.h"

using namespace jrpc;

void run(EchoClientStub& client)
{
    static int counter = 0;
    counter++;

    std::string str = "hello RpcServer +" + std::to_string(counter) + "s";
    client.Echo(str, [](json::Value response, bool isError, bool timeout) {
        if (!isError) {
            std::cout << "response: " << response.getStringView() << "\n";
        }
        else if (timeout) {
            std::cout << "timeout\n";
        }
        else {
            std::cout << "response: "
                      << response["message"].getStringView() << ": "
                      << response["data"].getStringView() << "\n";
        }
    });
}

int main()
{
    EventLoop loop;
    InetAddress serverAddr(9877);
    EchoClientStub client(&loop, serverAddr);

    client.setConnectionCallback([&](const TcpConnectionPtr& conn) {
        if (conn->disconnected()) {
            loop.quit();
        }
        else {
            loop.runEvery(100ms, [&] {
                run(client);
            });
        }
    });

    client.start();
    loop.loop();
}