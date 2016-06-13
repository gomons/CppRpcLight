#pragma once

#include <sstream>
#include <string>
#include <tuple>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include "FunctionCall.h"
#include "ServerConnection.h"
#include "TupleSerialization.h"

#define RPC_DEFINE(func_name, return_type, ...) \
    struct func_name##_descr { \
        typedef return_type ret_type; \
        typedef std::tuple<__VA_ARGS__> arg_type; \
    }; \
    return_type func_name(__VA_ARGS__)

#define RPC_DECLARE(func_name, return_type, ...) \
    struct func_name##_adder { \
        func_name##_adder() { \
            cpp_rpc_light::ServerConnection::GetRpcFunctions()[#func_name] = [] (const std::string &in) { \
                std::istringstream in_stream(in); \
                boost::archive::binary_iarchive in_archive(in_stream); \
                func_name##_descr::arg_type args; \
                in_archive >> args; \
                auto res = cpp_rpc_light::call(&func_name, args); \
                std::stringstream buffer_stream; \
                boost::archive::binary_oarchive out_archive(buffer_stream); \
                out_archive << res; \
                return buffer_stream.str(); \
            }; \
        } \
    } func_name##_adder_; \
    return_type func_name(__VA_ARGS__)