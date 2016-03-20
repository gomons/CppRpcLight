#include <iostream>
#include <thread>
#include <type_traits>
#include "CppRpcLight/ClientConnection.h"
#include "CppRpcLight/Pack.h"

namespace crl = cpp_rpc_light;
namespace ba = boost::asio;

#define RPC_DEFINE(func_name, return_type, ...) \
    template<typename... Args> \
    return_type func_name(Args... args) \
    { \
        std::enable_if<std::is_same<std::tuple<Args...>, std::tuple<__VA_ARGS__>>::value, bool>::type types_the_same; \
        auto future = ClientConnectionGetter::get().ExecuteFunction<return_type>(#func_name, args...); \
        return future.get(); \
    } \
    template<typename... Args> \
    std::future<return_type> func_name##_async(Args... args) {\
        std::enable_if<std::is_same<std::tuple<Args...>, std::tuple<__VA_ARGS__>>::value, bool>::type types_the_same; \
        return ClientConnectionGetter::get().ExecuteFunction<return_type>(#func_name, args...); \
    }
    

RPC_DEFINE(sum, int, int, int)

class ClientConnectionGetter
{
public:
    static crl::ClientConnection *client_connection;

    static crl::ClientConnection& get()
    {
        return *client_connection;
    }
};
crl::ClientConnection *ClientConnectionGetter::client_connection = nullptr;


//template<typename... Args>
//int sum(Args... args)
//{
//    auto future = ClientConnectionGetter::get().ExecuteFunction<int>("sum", args...);
//    return future.get();
//}

int main(int argc, char *argv[])
{
	std::cout << "Hello, RrcClient!" << std::endl;

	try
	{
		ba::io_service io_service;
		crl::ClientConnection clientConnection(io_service);
        ClientConnectionGetter::client_connection = &clientConnection;

		auto thread_func = [&io_service]() {
			std::cout << "Client started!" << std::endl;
			io_service.run();
			std::cout << "Client stopped!" << std::endl;
		};
		std::thread thread(thread_func);

		clientConnection.WaitForConnect();

		//std::future<int> result = clientConnection.ExecuteFunction<int>("sum", 10, 15);
		//auto val = result.get();

        auto val = sum(10, 25);
        auto val2 = sum(10, 26);
        //auto val_future = sum_async(10, 26);
        //auto val = val_future.get();

		thread.join();
	}
	catch (const std::exception &e)
	{
		std::cout << "Client error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}