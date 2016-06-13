#pragma once

#include <memory>
#include <boost/asio.hpp>
#include "PackHeader.h"

namespace cpp_rpc_light
{
    // Class is thread sage
    class ServerConnection: public std::enable_shared_from_this<ServerConnection>
    {
    public:
        static std::shared_ptr<ServerConnection> Create(boost::asio::io_service &io_service);
        static std::map<std::string, std::function<std::string(const std::string&)>> &GetRpcFunctions();

        void Start();
        boost::asio::ip::tcp::socket& socket();

    private:
        ServerConnection(boost::asio::io_service &io_service);

        void HandleReadPackHeader(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<boost::asio::streambuf> response_shared_streambuf);
        void HandleReadPack(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<boost::asio::streambuf> response_shared_streambuf, const RequestPackHeader &request_pack_header);
        void HandleWrite(const boost::system::error_code &error, size_t bytes_transferred);

        boost::asio::ip::tcp::socket socket_;
    };
} // namespace cpp_rpc_light