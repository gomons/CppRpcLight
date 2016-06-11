#include <iostream>
#include <thread>
#include <CppRpcLight/ClientConnection.h>
#include <RpcServer/Service.h>

int main(int argc, char *argv[])
{
	std::cout << "Hello, RrcClient!" << std::endl;
	try
	{
		boost::asio::io_service io_service;
		cpp_rpc_light::ClientConnection client_ñonnection(io_service);
		std::thread thread([&io_service]() {
            std::cout << "Client started!" << std::endl;
            io_service.run();
            std::cout << "Client stopped!" << std::endl;
        });
        client_ñonnection.WaitForConnect();

        auto val = sum(client_ñonnection, 10, 25);
        auto val2 = sum(client_ñonnection, 10, 26);
        auto val_future = sum_async(client_ñonnection, 10, 26);
        auto val3 = val_future.get();

		thread.join();
	}
	catch (const std::exception &e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}