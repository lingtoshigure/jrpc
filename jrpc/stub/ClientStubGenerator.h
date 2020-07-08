#pragma once

#include <jrpc/stub/StubGenerator.h>

namespace jrpc
{

class ClientStubGenerator: public StubGenerator
{
public:
    explicit
    ClientStubGenerator(json::Value& proto)
    : StubGenerator(proto)
    { }

    std::string genStub() override;
    std::string genStubClassName() override; 

private:
    std::string genProcedureDefinitions();
    std::string genNotifyDefinitions();

    template <typename Rpc>
    std::string genGenericArgs(const Rpc& r, bool appendComma);
    template <typename Rpc>
    std::string genGenericParamMembers(const Rpc& r);
};

}
