#pragma once

#include <unordered_map>
#include <string_view>
#include <memory>

#include <cppJson/Value.h>
#include <jrpc/server/Procedure.h>

namespace jrpc
{

/// @brief: 这是针对每个调用的函数相关服务的类
class RpcService: noncopyable
{
public:
    void addProcedureReturn(std::string_view methodName, ProcedureReturn* p)
    {
        assert(procedureReturn_.find(methodName) == procedureReturn_.end());
        procedureReturn_.emplace(methodName, p);
    }

    void addProcedureNotify(std::string_view methodName, ProcedureNotify *p)
    {
        assert(procedureNotfiy_.find(methodName) == procedureNotfiy_.end());
        procedureNotfiy_.emplace(methodName, p);
    }

    void callProcedureReturn(std::string_view methodName,
                             json::Value& request,
                             const RpcDoneCallback& done);

    void callProcedureNotify(std::string_view methodName, 
                             json::Value& request);

private:
    // 根据函数名 - 函数调用, 建立映射关系
    std::unordered_map<std::string_view, std::unique_ptr<ProcedureReturn>> procedureReturn_;
    std::unordered_map<std::string_view, std::unique_ptr<ProcedureNotify>> procedureNotfiy_;
};


}
