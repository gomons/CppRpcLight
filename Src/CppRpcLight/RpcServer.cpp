#include "RpcServer.h"

namespace cpp_rpc_light
{
    RpcServer::RpcServer(boost::asio::io_service &io_service)
        : acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 12345))
    {
        StartAccept();
    }

    void RpcServer::StartAccept()
    {
        auto new_connection = ServerConnection::Create(acceptor_.get_io_service());
        auto &socket = new_connection->socket();
        auto accept_handler = [this, new_connection](const boost::system::error_code &error) {
            HandleAccept(new_connection, error);
        };
        acceptor_.async_accept(socket, accept_handler);
    }

    void RpcServer::HandleAccept(std::shared_ptr<ServerConnection> new_connection, const boost::system::error_code& error)
    {
        if (error)
            throw boost::system::system_error(error);
        new_connection->Start();
        StartAccept();
    }
} // namespace cpp_rpc_light