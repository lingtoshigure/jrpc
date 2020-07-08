#pragma once

#include <cppJson/Value.h>
#include <jrpc/util.h>

namespace jrpc
{

using ProcedureReturnCallback = std::function<void(json::Value&, const RpcDoneCallback&)> ;  // 完成时候的回调函数
using ProcedureNotifyCallback = std::function<void(json::Value&)> ;

template <typename Func>
class Procedure: noncopyable
{
public:
    template<typename... ParamNameAndTypes>
    explicit Procedure(Func&& callback, ParamNameAndTypes&&... nameAndTypes)
    : callback_(std::forward<Func>(callback))
    {
        constexpr int n = sizeof...(nameAndTypes);
        // 名字和参数需要成对出现
        static_assert(n % 2 == 0, "procedure must have param name and type pairs");

        if constexpr (n > 0)
            initProcedure(nameAndTypes...);
    }

    // procedure call
    void invoke(json::Value& request, const RpcDoneCallback& done);
    // procedure notify
    void invoke(json::Value& request);

private:
    template<typename Name, typename Type, typename... ParamNameAndTypes>
    void initProcedure(Name paramName, Type parmType, ParamNameAndTypes &&... nameAndTypes)
    {
        static_assert(std::is_same_v<Type, json::ValueType>, "param type must be json::ValueType");
    }

    ///@biref: 这是一个特化版本,也应该是这个版本,如果传入的参数类型不是 json::ValueType, 就会触发上面那个函数的 static_assert
    ///@biref: 这个函数的作用就是将 <参数名,参数> 成对的放到 @b params_ 中
    template<typename Name, typename... ParamNameAndTypes>
    void initProcedure(Name paramName, json::ValueType parmType, ParamNameAndTypes &&... nameAndTypes)
    {
        static_assert(std::is_same_v<Name, const char *> ||
                      std::is_same_v<Name, std::string_view>,
                      "param name must be 'const char*' or 'std::string_view'");
        params_.emplace_back(paramName, parmType);
        if constexpr (sizeof...(ParamNameAndTypes) > 0)
            initProcedure(nameAndTypes...);
    }

    void validateRequest(json::Value& request) const;
    bool validateGeneric(json::Value& request) const;

private:
    // 参数包含：参数名，参数类型
    struct Param
    {
        Param(std::string_view paramName_, json::ValueType paramType_) 
        : paramName(paramName_),
          paramType(paramType_)
        {}

        std::string_view paramName;
        json::ValueType  paramType;
    };

private:
    // 一个可执行程序的返回时的回调函数
    // 这个函数的参数
    Func callback_;
    std::vector<Param> params_;
};


using ProcedureReturn = Procedure<ProcedureReturnCallback>;
using ProcedureNotify = Procedure<ProcedureNotifyCallback>;

}
