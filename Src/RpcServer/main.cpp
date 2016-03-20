#include <iostream>
#include <map>
#include <string>
#include <functional>
#include <tuple>
#include <cstdint>
#include <sstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace boost {
    namespace serialization {
        template<uint32_t N>
        struct Serialize
        {
            template<class Archive, typename... Args>
            static void serialize(Archive & ar, std::tuple<Args...> & t, const unsigned int version)
            {
                ar & std::get<N - 1>(t);
                Serialize<N - 1>::serialize(ar, t, version);
            }
        };

        template<>
        struct Serialize<0>
        {
            template<class Archive, typename... Args>
            static void serialize(Archive & ar, std::tuple<Args...> & t, const unsigned int version)
            {
            }
        };

        template<class Archive, typename... Args>
        void serialize(Archive & ar, std::tuple<Args...> & t, const unsigned int version)
        {
            Serialize<sizeof...(Args)>::serialize(ar, t, version);
        }
    }
}

namespace 
{
    template<typename F, typename Tuple, bool Enough, int TotalArgs, int... N>
    struct call_impl
    {
        auto static call(F f, Tuple&& t)
        {
            return call_impl<F, Tuple, TotalArgs == 1 + sizeof...(N),
                TotalArgs, N..., sizeof...(N)
            >::call(f, std::forward<Tuple>(t));
        }
    };

    template<typename F, typename Tuple, int TotalArgs, int... N>
    struct call_impl<F, Tuple, true, TotalArgs, N...>
    {
        auto static call(F f, Tuple&& t)
        {
            return f(std::get<N>(std::forward<Tuple>(t))...);
        }
    };

    template<typename F, typename Tuple>
    auto call(F f, Tuple&& t)
    {
        typedef typename std::decay<Tuple>::type type;
        return call_impl<F, Tuple, 0 == std::tuple_size<type>::value,
            std::tuple_size<type>::value
        >::call(f, std::forward<Tuple>(t));
    }
}



std::map<std::string, std::function<std::string(const std::string&)>> rpc_functions;


#define RPC_DEFINE(func_name, return_type, ...) \
    struct func_name##_descr { \
        typedef return_type ret_type; \
        typedef std::tuple<__VA_ARGS__> arg_type; \
    }; \
    return_type func_name(__VA_ARGS__);

#define RPC_DECLARE(func_name, return_type, ...) \
    struct func_name##adder { \
        func_name##adder() { \
            rpc_functions[#func_name] = [] (const std::string &in) { \
                std::istringstream in_stream(in); \
                boost::archive::binary_iarchive in_archive(in_stream); \
                func_name##_descr::arg_type args; \
                in_archive >> args; \
                auto res = call(&func_name, args); \
                std::stringstream buffer_stream; \
                boost::archive::binary_oarchive out_archive(buffer_stream); \
                out_archive << res; \
                return buffer_stream.str(); \
            }; \
        } \
    } func_name##adder_; \
    return_type func_name(__VA_ARGS__)

RPC_DEFINE(sum, int, int, int)
RPC_DECLARE(sum, int, int a, int b) {
    return a + b;
}

#include <CppRpcLight/RpcServer.h>

int main(int argc, char *argv[])
{

    try 
    {
        using namespace cpp_rpc_light;

        boost::asio::io_service io_service;
        RpcServer prc_server(io_service);
        io_service.run();
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;


    //sum_descr::ret_type i;
    sum_descr::arg_type args(10, 15);

    // Это на стороне клиента. На вход - тупл, он сериализуется и передается на другую сторону.
    std::stringstream buffer_stream;
    boost::archive::binary_oarchive out_archive(buffer_stream);
    out_archive << args;
    auto buffer_ptr = std::make_shared<std::string>();
    *buffer_ptr += buffer_stream.str();

    try {
        // Это происходит на стороне сервера. Получили из сети данные buffer_ptr
        // По имени функции нашли ее
        // После запуска функции получили сериализованный результат

        auto func = rpc_functions.at("sum");
        auto res = func(*buffer_ptr);
        std::istringstream in_stream(res);
        boost::archive::binary_iarchive in_archive(in_stream);
        sum_descr::ret_type ret;
        in_archive >> ret;
        int i = 0;
    }
    catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }

    auto res = sum(10, 15);

	return 0;
}