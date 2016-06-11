#pragma once

#include <memory>
#include <boost/asio.hpp>
#include "PackHeader.h"

namespace cpp_rpc_light
{
    class ServerConnection: public std::enable_shared_from_this<ServerConnection>
    {
    public:
        typedef std::shared_ptr<ServerConnection> Ptr;

        static Ptr Create(boost::asio::io_service &io_service);

        void Start();
        boost::asio::ip::tcp::socket& socket() { return socket_; }

    private:
        ServerConnection(boost::asio::io_service &io_service);

        void HandleReadPackHeader(const boost::system::error_code &error, size_t bytes_transferred);
        void HandleReadPack(const boost::system::error_code &error, size_t bytes_transferred, const RequestPackHeader &request_pack_header);
        void HandleWrite(const boost::system::error_code &error, size_t bytes_transferred);

        boost::asio::ip::tcp::socket socket_;
        boost::asio::streambuf response_streambuf_;

    public:

        static std::map<std::string, std::function<std::string(const std::string&)>> &GetRpcFunctions()
        {
            static std::map<std::string, std::function<std::string(const std::string&)>> rpc_functions;
            return rpc_functions;
        }
    };
} // namespace cpp_rpc_light