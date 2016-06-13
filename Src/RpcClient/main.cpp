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
        while (true) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            auto echo_response = echo_async(client_ñonnection, std::string("Ping!"));
            std::cout << echo_response.get() << std::endl;
        }
        thread.join();
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}