#include <jrpc/stub/ServiceStubGenerator.h>

using namespace jrpc;

namespace
{

// 服务端的参数模型
/**
 * @userClassName: EchoService
 * @stubClassName: EchoServiceStub
 * @serviceName  : Echo
 * @stubProcedureBindings: 
        service->addProcedureReturn("Echo", 
                                    new ProcedureReturn(
                                        std::bind(&EchoServiceStub::EchoStub, this, _1, _2), 
                                        "message", json::TYPE_STRING
                                    ));
 *@stubProcedureDefinitions：
        void EchoStub(json::Value& request, const RpcDoneCallback& done)
        {
            auto& params = request["params"];

            if (params.isArray()) {
                auto message = params[0].getString();

                convert().Echo(message,  UserDoneCallback(request, done));
            }
            else 
            {
                auto message = params["message"].getString();

                convert().Echo(message,  UserDoneCallback(request, done));
            }
        }
*/
std::string serviceStubTemplate(const std::string& userClassName,
                                const std::string& stubClassName,
                                const std::string& serviceName,
                                const std::string& stubProcedureBindings,
                                const std::string& stubProcedureDefinitions)
{
    std::string str =
R"(
/*
 * This stub is generated by jrpc, DO NOT modify it!
 */
#pragma once

#include <cppJson/Value.h>

#include <jrpc/util.h>
#include <jrpc/server/RpcServer.h>
#include <jrpc/server/RpcService.h>

class [userClassName];

namespace jrpc
{

template <typename S>
class [stubClassName]: noncopyable
{
protected:
    explicit
    [stubClassName](RpcServer& server)
    {
        static_assert(std::is_same_v<S, [userClassName]>,
                      "derived class name should be '[userClassName]'");

        auto service = new RpcService;

        [stubProcedureBindings]

        server.addService("[serviceName]", service);
    }

    ~[stubClassName]() = default;

private:
    [stubProcedureDefinitions]

private:
    S& convert()
    {
        return static_cast<S&>(*this);
    }
};

}
)";

    replaceAll(str, "[userClassName]",              userClassName);
    replaceAll(str, "[stubClassName]",              stubClassName);
    replaceAll(str, "[serviceName]",                serviceName);
    replaceAll(str, "[stubProcedureBindings]",      stubProcedureBindings);
    replaceAll(str, "[stubProcedureDefinitions]",   stubProcedureDefinitions);
    return str;
}

/**
 * @procedureName    : Echo
 * @stubClassName    : EchoServiceStub
 * @stubProcedureName: EchoStub
 * @procedureParams  :    , "message", json::TYPE_STRING

    service->addProcedureReturn("Echo", 
                                new ProcedureReturn(
                                    std::bind(&EchoServiceStub::EchoStub, this, _1, _2), 
                                    "message", 
                                    json::TYPE_STRING
                                ));
*/
std::string stubProcedureBindTemplate(const std::string& procedureName,
                                      const std::string& stubClassName,
                                      const std::string& stubProcedureName,
                                      const std::string& procedureParams)
{
    // void addProcedureReturn(std::string_view methodName, ProcedureReturn* p)
    std::string str =
        R"(
        service->addProcedureReturn("[procedureName]", 
                                    new ProcedureReturn(
                                        std::bind(&[stubClassName]::[stubProcedureName], this, _1, _2)
                                        [procedureParams]
                                    ));
        )";

    replaceAll(str, "[procedureName]",      procedureName);
    replaceAll(str, "[stubClassName]",      stubClassName);
    replaceAll(str, "[stubProcedureName]",  stubProcedureName);
    replaceAll(str, "[procedureParams]",    procedureParams);
    return str;
}

std::string stubNotifyBindTemplate(const std::string& notifyName,
                                   const std::string& stubClassName,
                                   const std::string& stubNotifyName,
                                   const std::string& notifyParams)
{
    std::string str =
        R"(
        service->addProcedureNotify("[notifyName]", 
                                    new ProcedureNotify(
                                        std::bind(&[stubClassName]::[stubNotifyName], this, _1)
                                        [notifyParams]
                                    ));
        )";

    replaceAll(str, "[notifyName]",     notifyName);
    replaceAll(str, "[stubClassName]",  stubClassName);
    replaceAll(str, "[stubNotifyName]", stubNotifyName);
    replaceAll(str, "[notifyParams]",   notifyParams);
    return str;
}

// 回调函数
/**
 * @paramsFromJsonArray：auto message = params[0].getString();
 * @paramsFromJsonObject：auto message = params["message"].getString();
 * @stubProcedureName：EchoStub
 * @procedureName：Echo
 * @procedureArgs: message
 * 
 * 
 * 
 *  void EchoStub(json::Value& request, const RpcDoneCallback& done)
    {
        auto& params = request["params"];

        if (params.isArray()) {
            auto message = params[0].getString();

            convert().Echo(message,  UserDoneCallback(request, done));
        }
        else {
            auto message = params["message"].getString();

            convert().Echo(message,  UserDoneCallback(request, done));
        }
    }
*/
std::string stubProcedureDefineTemplate(const std::string& paramsFromJsonArray,
                                        const std::string& paramsFromJsonObject,
                                        const std::string& stubProcedureName,
                                        const std::string& procedureName,
                                        const std::string& procedureArgs)
{
   std::string str =
    R"(void [stubProcedureName](json::Value& request, const RpcDoneCallback& done)
    {
        auto& params = request["params"];

        if (params.isArray()) {
            [paramsFromJsonArray]
            convert().[procedureName]([procedureArgs] UserDoneCallback(request, done));
        }
        else {
            [paramsFromJsonObject]
            convert().[procedureName]([procedureArgs] UserDoneCallback(request, done));
        }
    })";

    replaceAll(str, "[paramsFromJsonArray]", paramsFromJsonArray);
    replaceAll(str, "[paramsFromJsonObject]", paramsFromJsonObject);
    replaceAll(str, "[stubProcedureName]", stubProcedureName);
    replaceAll(str, "[procedureName]", procedureName);
    replaceAll(str, "[procedureArgs]", procedureArgs);
    return str;
}

std::string stubProcedureDefineTemplate(const std::string& stubProcedureName,
                                        const std::string& procedureName)
{
    std::string str =
R"(
void [stubProcedureName](json::Value& request, const RpcDoneCallback& done)
{
    convert().[procedureName](UserDoneCallback(request, done));
}
)";

    replaceAll(str, "[stubProcedureName]", stubProcedureName);
    replaceAll(str, "[procedureName]", procedureName);
    return str;
}

std::string stubNotifyDefineTemplate(const std::string& paramsFromJsonArray,
                                     const std::string& paramsFromJsonObject,
                                     const std::string& stubNotifyName,
                                     const std::string& notifyName,
                                     const std::string& notifyArgs)
{
    std::string str =
R"(
void [stubNotifyName](json::Value& request)
{
    auto& params = request["params"];

    if (params.isArray()) {
        [paramsFromJsonArray]
        convert().[NotifyName]([notifyArgs]);
    }
    else {
        [paramsFromJsonObject]
        convert().[NotifyName]([notifyArgs]);
    }
}
)";

    replaceAll(str, "[notifyName]", notifyName);
    replaceAll(str, "[stubNotifyName]", stubNotifyName);
    replaceAll(str, "[notifyArgs]", notifyArgs);
    replaceAll(str, "[paramsFromJsonArray]", paramsFromJsonArray);
    replaceAll(str, "[paramsFromJsonObject]", paramsFromJsonObject);
    return str;
}

std::string stubNotifyDefineTemplate(const std::string& stubNotifyName,
                                     const std::string& notifyName)
{
    std::string str =
    R"(
        void [stubNotifyName](json::Value& request)
        {
            convert().[notifyName]();
        }
    )";

    replaceAll(str, "[stubNotifyName]", stubNotifyName);
    replaceAll(str, "[notifyName]", notifyName);
    return str;
}

std::string argsDefineTemplate(const std::string& arg,
                               const std::string& index,
                               json::ValueType    type)
{
    std::string str = R"(auto [arg] = params[[index]][method];)";
    std::string method = [=]
                         {
                            switch (type) {
                                case json::TYPE_BOOL:
                                    return ".getBool()";
                                case json::TYPE_INT32:
                                    return ".getInt32()";
                                case json::TYPE_INT64:
                                    return ".getInt64()";
                                case json::TYPE_DOUBLE:
                                    return ".getDouble()";
                                case json::TYPE_STRING:
                                    return ".getString()";
                                case json::TYPE_OBJECT:
                                case json::TYPE_ARRAY:
                                    return "";//todo
                                default:
                                    assert(false && "bad value type");
                                    return "bad type";
                            }
                         }();
    replaceAll(str, "[arg]", arg);
    replaceAll(str, "[index]", index);
    replaceAll(str, "[method]", method);
    return str;
}

} //unnamed - namespace

// 生成 xxx_stub.h 文件
std::string ServiceStubGenerator::genStub()
{
    auto  userClassName = genUserClassName();
    auto  stubClassName = genStubClassName();
    auto& serviceName   = serviceInfo_.name;

    auto bindings = genStubProcedureBindings();
    bindings.append(genStubNotifyBindings());

    auto definitions = genStubProcedureDefinitions();
    definitions.append(genStubNotifyDefinitions());

    return serviceStubTemplate(userClassName,
                               stubClassName,
                               serviceName,
                               bindings,
                               definitions);
}

std::string ServiceStubGenerator::genUserClassName()
{
    return serviceInfo_.name + "Service";    // EchoService
}

std::string ServiceStubGenerator::genStubClassName()
{
    return serviceInfo_.name + "ServiceStub"; // EchoStub
}   

// 设置回调函数
std::string ServiceStubGenerator::genStubProcedureBindings()
{
    std::string result;
    for (RpcReturn& p: serviceInfo_.rpcReturn) {
        auto procedureName      = p.name;
        auto stubClassName      = genStubClassName();
        auto stubProcedureName  = genStubGenericName(p);
        auto procedureParams    = genGenericParams(p);

        auto binding = stubProcedureBindTemplate(procedureName,         // Echo
                                                 stubClassName,         // 类名：EchoServiceStub
                                                 stubProcedureName,     // 内部的回调函数， EchoStub
                                                 procedureParams);      // EchoStub 参数
        result.append(binding);
        result.append("\n");
    }
    return result;
}

// 填回调函数
std::string ServiceStubGenerator::genStubProcedureDefinitions()
{
    std::string result;
    for (RpcReturn& r: serviceInfo_.rpcReturn) {
        auto procedureName     = r.name;
        auto stubProcedureName = genStubGenericName(r);

        if (r.params.getSize() > 0) {
            auto paramsFromJsonArray  = genParamsFromJsonArray(r);
            auto paramsFromJsonObject = genParamsFromJsonObject(r);
            auto procedureArgs        = genGenericArgs(r);
            auto define               = stubProcedureDefineTemplate(paramsFromJsonArray,
                                                                    paramsFromJsonObject,
                                                                    stubProcedureName,
                                                                    procedureName,
                                                                    procedureArgs);

            result.append(define);
            result.append("\n");
        }
        else {
            auto define = stubProcedureDefineTemplate(stubProcedureName,procedureName);

            result.append(define);
            result.append("\n");
        }
    }
    return result;
}

std::string ServiceStubGenerator::genStubNotifyBindings()
{
    std::string result;
    for (RpcNotify& p: serviceInfo_.rpcNotify) {
        auto notifyName     = p.name;
        auto stubClassName  = genStubClassName();
        auto stubNotifyName = genStubGenericName(p);
        auto notifyParams   = genGenericParams(p);

        auto binding = stubNotifyBindTemplate(notifyName,
                                              stubClassName,
                                              stubNotifyName,
                                              notifyParams);
        result.append(binding);
        result.append("\n");
    }
    return result;
}

std::string ServiceStubGenerator::genStubNotifyDefinitions()
{
    std::string result;
    for (auto& r: serviceInfo_.rpcNotify) {
        auto notifyName = r.name;
        auto stubNotifyName = genStubGenericName(r);

        if (r.params.getSize() > 0) {
            auto paramsFromJsonArray = genParamsFromJsonArray(r);
            auto paramsFromJsonObject = genParamsFromJsonObject(r);
            auto notifyArgs = genGenericArgs(r);
            auto define = stubNotifyDefineTemplate(
                    paramsFromJsonArray,
                    paramsFromJsonObject,
                    stubNotifyName,
                    notifyName,
                    notifyArgs);

            result.append(define);
            result.append("\n");
        }
        else {
            auto define = stubNotifyDefineTemplate(
                    stubNotifyName,
                    notifyName);

            result.append(define);
            result.append("\n");
        }
    }
    return result;
}

template <typename Rpc>
std::string ServiceStubGenerator::genStubGenericName(const Rpc& r)
{
    return r.name + "Stub";
}

template <typename Rpc>
std::string ServiceStubGenerator::genGenericParams(const Rpc& r)
{
    std::string result;

    for (auto& m: r.params.getObject()) {
        std::string field = "\"" + m.key.getString()+"\""; // "message"
        std::string type  = [&] 
                            {
                            switch (m.value.getType()) {
                                case json::TYPE_BOOL:   return "json::TYPE_BOOL";
                                case json::TYPE_INT32:  return "json::TYPE_INT32";
                                case json::TYPE_INT64:  return "json::TYPE_INT64";
                                case json::TYPE_DOUBLE: return "json::TYPE_DOUBLE";
                                case json::TYPE_STRING: return "json::TYPE_STRING";
                                case json::TYPE_OBJECT: return "json::TYPE_OBJECT";
                                case json::TYPE_ARRAY:  return "json::TYPE_ARRAY";
                                default: assert(false && "bad value type"); return "bad type";
                            };
                                
                               
                            }();
        result.append(", ").append(field);
        result.append(", ").append(type);
    }
    return result;
}

template <typename Rpc>
std::string ServiceStubGenerator::genGenericArgs(const Rpc& r)
{
    // 所有的参数都是以 , 分割
    std::string result;
    for (auto& m: r.params.getObject()) {
        auto arg = m.key.getString();
        result.append(arg);
        result.append(", ");
    }
    return result;
}

template <typename Rpc>
std::string ServiceStubGenerator::genParamsFromJsonArray(const Rpc& r)
{
    // auto message = params[0].getString();
    std::string result;
    int index = 0;
    for (auto& m: r.params.getObject()) {
        std::string line = argsDefineTemplate(m.key.getString(),     // arg
                                              std::to_string(index), // index
                                              m.value.getType());    // type --> method
        index++;
        result.append(line);
        result.append("\n");
    }
    return result;
}

template <typename Rpc>
std::string ServiceStubGenerator::genParamsFromJsonObject(const Rpc& r)
{
    std::string result;
    for (auto& m: r.params.getObject()) {
        std::string index = "\"" + m.key.getString() + "\"";
        std::string line = argsDefineTemplate(m.key.getString(),
                                              index,
                                              m.value.getType());
        result.append(line);
        result.append("\n");
    }
    return result;
}