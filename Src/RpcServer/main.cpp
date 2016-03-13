#include <iostream>
#include "CppRpcLight/RpcNetworkProcessor.h"

namespace crl = cpp_rpc_light;
namespace ba = boost::asio;

int main(int argc, char *argv[])
{
	std::cout << "Hello, Server!" << std::endl;

	try
	{
		ba::io_service io_service;
		crl::RpcNetworkProcessor rpcNetworkProcessor(io_service);
		std::cout << "Server started!" << std::endl;
		io_service.run();
		std::cout << "Server stopped!" << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cout << "Server error: " << e.what() << std::endl;
	}

	return 0;
}