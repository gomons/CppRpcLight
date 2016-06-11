#pragma once

#include <future>
#include "ClientConnection.h"

#define RPC_DEFINE(func_name, return_type, ...) \
    template<typename... Args> \
    std::future<return_type> func_name##_async(cpp_rpc_light::ClientConnection &client_connection, Args... args) {\
        return client_connection.ExecuteFunction<return_type>(#func_name, args...); \
    } \
    template<typename... Args> \
    return_type func_name(cpp_rpc_light::ClientConnection &client_connection, Args... args) \
    { \
        auto future = func_name##_async(client_connection, args...); \
        return future.get(); \
    }