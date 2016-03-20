#pragma once

#include <memory>
#include <boost/asio.hpp>
#include "aliases.h"
#include "PackHeader.h"

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

		void HandleReadPackHeader(const bs::error_code &error, size_t bytes_transferred);
		void HandleReadPack(const bs::error_code &error, size_t bytes_transferred, const RequestPackHeader &request_pack_header);
		void HandleWrite(const bs::error_code &error, size_t bytes_transferred);

		ba_tcp::socket socket_;
		ba::streambuf response_streambuf_;

    public:
        static std::map<std::string, std::function<std::string(const std::string&)>> rpc_functions;
	};
} // namespace cpp_rpc_light