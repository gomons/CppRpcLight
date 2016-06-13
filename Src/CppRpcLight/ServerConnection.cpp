#include "ServerConnection.h"
#include <cstdint>
#include "FunctionCall.h"
#include "ServerMacro.h"
#include "TupleSerialization.h"

namespace cpp_rpc_light
{
    std::shared_ptr<ServerConnection> ServerConnection::Create(boost::asio::io_service &io_service)
    {
        return std::shared_ptr<ServerConnection>(new ServerConnection(io_service));
    }

    std::map<std::string, std::function<std::string(const std::string&)>> &ServerConnection::GetRpcFunctions()
    {
        static std::map<std::string, std::function<std::string(const std::string&)>> rpc_functions;
        return rpc_functions;
    }

    ServerConnection::ServerConnection(boost::asio::io_service &io_service) 
        : socket_(io_service)
    {}

    void ServerConnection::Start()
    {
        auto shared_this = shared_from_this();
        auto response_shared_streambuf = std::make_shared<boost::asio::streambuf>();
        auto read_pack_size_handler = [shared_this, response_shared_streambuf](const boost::system::error_code &error, size_t bytes_transferred) {
            shared_this->HandleReadPackHeader(error, bytes_transferred, response_shared_streambuf);
        };
        boost::asio::async_read(socket_, *response_shared_streambuf, boost::asio::transfer_exactly(sizeof(RequestPackHeader)), read_pack_size_handler);
    }

    boost::asio::ip::tcp::socket& ServerConnection::socket() 
    { 
        return socket_; 
    }

    void ServerConnection::HandleReadPackHeader(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<boost::asio::streambuf> response_shared_streambuf)
    {
        if (error)
            throw boost::system::system_error(error);

        auto in_buffer = response_shared_streambuf->data();
        auto request_pack_header = *reinterpret_cast<const RequestPackHeader*>(&*boost::asio::buffers_begin(in_buffer));
        response_shared_streambuf->consume(bytes_transferred);

        auto shared_this = shared_from_this();
        auto read_pack_size_handler = [shared_this, response_shared_streambuf, request_pack_header](const boost::system::error_code &error, size_t bytes_transferred) {
            shared_this->HandleReadPack(error, bytes_transferred, response_shared_streambuf, request_pack_header);
        };
        boost::asio::async_read(socket_, *response_shared_streambuf, boost::asio::transfer_exactly(request_pack_header.arg_length), read_pack_size_handler);
    }

    void ServerConnection::HandleReadPack(const boost::system::error_code &error, size_t bytes_transferred, std::shared_ptr<boost::asio::streambuf> response_shared_streambuf, const RequestPackHeader &request_pack_header)
    {
        if (error)
            throw boost::system::system_error(error);

        std::istream in_stream(response_shared_streambuf.get());
        std::string buffer;
        buffer.resize(bytes_transferred);
        in_stream.read(&*buffer.begin(), bytes_transferred);

        auto res = GetRpcFunctions()[request_pack_header.func_name](buffer);

        ResponsePackHeader response_pack_header;
        response_pack_header.call_id = request_pack_header.call_id;
        response_pack_header.arg_length = res.size();

        std::string header_buffer(reinterpret_cast<const char*>(&response_pack_header), sizeof(response_pack_header));
        auto buffer_ptr = std::make_shared<std::string>();
        *buffer_ptr += header_buffer + res;
        auto out_buffer = boost::asio::buffer(buffer_ptr->data(), buffer_ptr->size());

        auto shared_this = shared_from_this();
        auto write_handler = [shared_this, buffer_ptr](const boost::system::error_code &error, size_t bytes_transferred) {
            shared_this->HandleWrite(error, bytes_transferred);
        };
        boost::asio::async_write(socket_, out_buffer, write_handler);
    }

    void ServerConnection::HandleWrite(const boost::system::error_code &error, size_t bytes_transferred)
    {
        if (error)
            throw boost::system::system_error(error);
        Start();
    }
} // namespace cpp_rpc_light