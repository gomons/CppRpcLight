#include <iostream>
#include <thread>
#include "CppRpcLight/ClientConnection.h"

namespace crl = cpp_rpc_light;
namespace ba = boost::asio;

int main(int argc, char *argv[])
{
	std::cout << "Hello, RrcClient!" << std::endl;

	try
	{
		ba::io_service io_service;
		crl::ClientConnection clientConnection(io_service);

		auto thread_func = [&io_service]() {
			std::cout << "Client started!" << std::endl;
			io_service.run();
			std::cout << "Client stopped!" << std::endl;
		};
		std::thread thread(thread_func);

		clientConnection.WaitForConnect();
		auto result = clientConnection.ExecuteFunction<std::string>("TestFunc");
		auto val = result.get();

		thread.join();
	}
	catch (const std::exception &e)
	{
		std::cout << "Client error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}