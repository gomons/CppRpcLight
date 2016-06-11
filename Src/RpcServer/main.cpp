#include <iostream>
#include <CppRpcLight/RpcServer.h>

int main(int argc, char *argv[])
{
    try
    {
        std::cout << "Hello, RPC server!" << std::endl;
        using namespace cpp_rpc_light;
        boost::asio::io_service io_service;
        RpcServer prc_server(io_service);
        std::cout << "RPC server started!" << std::endl;
        io_service.run();
        std::cout << "RPC server stopped!" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
}