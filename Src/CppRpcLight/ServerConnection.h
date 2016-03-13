#pragma once

#include <memory>
#include <boost/asio.hpp>
#include "aliases.h"

namespace cpp_rpc_light
{
	class ServerConnection: public std::enable_shared_from_this<ServerConnection>
	{
	public:
		typedef std::shared_ptr<ServerConnection> Ptr;

		static Ptr Create(ba::io_service &io_service);

		void Start();
		ba_tcp::socket& socket() { return socket_; }

	private:
		ServerConnection(ba::io_service &io_service);

		void HandleReadPackSize(const bs::error_code &error, size_t bytes_transferred);
		void HandleReadPack(const bs::error_code &error, size_t bytes_transferred);
		void HandleWrite(const bs::error_code &error, size_t bytes_transferred);

		ba_tcp::socket socket_;
		ba::streambuf response_streambuf_;
	};
} // namespace cpp_rpc_light