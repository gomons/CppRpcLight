#pragma once

#include <boost/asio.hpp>
#include "ServerConnection.h"

namespace cpp_rpc_light
{
	class RpcServer
	{
	public:
		RpcServer(boost::asio::io_service &io_service);

	private:
		void StartAccept();
		void HandleAccept(ServerConnection::Ptr new_connection, const boost::system::error_code &error);

		boost::asio::ip::tcp::acceptor acceptor_;
	};
} // namespace cpp_rpc_light