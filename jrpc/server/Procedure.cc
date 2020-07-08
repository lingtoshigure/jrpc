#include <jrpc/Exception.h>
#include <jrpc/server/Procedure.h>

using namespace jrpc;

namespace jrpc
{

template class Procedure<ProcedureReturnCallback>; // 传入的回调哈函数类型只能是这两
template class Procedure<ProcedureNotifyCallback>;

}

template <>
void Procedure<ProcedureReturnCallback>::validateRequest(json::Value& request) const
{
    switch (request.getType()) {
        case json::TYPE_OBJECT:
        case json::TYPE_ARRAY:
            if (!validateGeneric(request))
                throw RequestException(RPC_INVALID_PARAMS,
                                       request["id"],
                                       "params name or type mismatch");
            break;
        default:
            throw RequestException(RPC_INVALID_REQUEST,
                                   request["id"],
                                   "params type must be object or array");
    }
}

template <>
void Procedure<ProcedureNotifyCallback>::validateRequest(json::Value& request) const
{
    switch (request.getType()) {
        case json::TYPE_OBJECT:
        case json::TYPE_ARRAY:
            if (!validateGeneric(request))
                throw NotifyException(RPC_INVALID_PARAMS,
                                      "params name or type mismatch");
            break;
        default:
            throw NotifyException(RPC_INVALID_REQUEST,
                                  "params type must be object or array");
    }
}

/// @biref: 检验参数是否符合规则
/// @return: 符合返回 @c true, 否则返回 @c false
template <typename Func>
bool Procedure<Func>::validateGeneric(json::Value& request) const
{   
    // 检验下这个调用的参数是否符合规则
    auto mIter = request.findMember("params");
    if (mIter == request.memberEnd()) {
        return params_.empty();
    }
    // "params" : params
    auto& params = mIter->value;
    if (params.getSize() == 0 || params.getSize() != params_.size()) {
        return false;
    }
    switch (params.getType()) {
        /// 如果是数组类型,那么这次 @c request 得到的参数类型 和 @c params_ 中应该逐个相同
        case json::TYPE_ARRAY:
            for (size_t i = 0; i < params_.size(); i++) {
                if (params[i].getType() != params_[i].paramType)
                    return false;
            }
            break;
        case json::TYPE_OBJECT:
        /// 如果是对象类型,那么首先要 @c params_ 每个参数都可以在 @c request 中找到 && 参数类型形态相同
            for (auto& p : params_) {
                auto it = params.findMember(p.paramName);
                if (it == params.memberEnd() || it->value.getType() != p.paramType)
                    return false;
            }
            break;
        default:
            return false;
    }
    return true;
}

template <>
void Procedure<ProcedureReturnCallback>::invoke(json::Value& request,
                                                const RpcDoneCallback& done)
{
    validateRequest(request);
    // 这个是任务完成的回调函数
    // 这个 callback_ 实际上就是 echoserver 中的 EchoStub
    callback_(request, done);
}

template <>
void Procedure<ProcedureNotifyCallback>::invoke(json::Value& request)
{
    validateRequest(request);
    callback_(request); // request 从请求变成了 response
}