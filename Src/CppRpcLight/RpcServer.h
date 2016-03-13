#pragma once

#include <boost/asio.hpp>
#include "ServerConnection.h"
#include "aliases.h"

namespace cpp_rpc_light
{
	class RpcServer
	{
	public:
		RpcServer(ba::io_service &io_service);

	private:
		void StartAccept();
		void HandleAccept(ServerConnection::Ptr new_connection, const bs::error_code &error);

		ba_tcp::acceptor acceptor_;
	};

} // namespace cpp_rpc_light