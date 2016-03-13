#include "RpcServer.h"

namespace cpp_rpc_light
{
	RpcServer::RpcServer(ba::io_service &io_service)
		: acceptor_(io_service, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), 12345))
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

	void RpcServer::HandleAccept(ServerConnection::Ptr new_connection, const bs::error_code& error)
	{
		if (error)
			throw bs::system_error(error);

		new_connection->Start();
		StartAccept();
	}
} // namespace cpp_rpc_light