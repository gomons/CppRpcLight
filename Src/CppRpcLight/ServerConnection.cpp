#include "ServerConnection.h"
#include <cstdint>
#include "TupleSerialization.h"
#include "FunctionCall.h"

#include "ServerMacro.h"

namespace cpp_rpc_light
{
	ServerConnection::Ptr ServerConnection::Create(boost::asio::io_service &io_service)
	{
		return Ptr(new ServerConnection(io_service));
	}

	ServerConnection::ServerConnection(boost::asio::io_service &io_service) 
		: socket_(io_service)
	{}

	void ServerConnection::Start()
	{
		auto shared_this = shared_from_this();
		auto read_pack_size_handler = [shared_this](const boost::system::error_code &error, size_t bytes_transferred) {
			shared_this->HandleReadPackHeader(error, bytes_transferred);
		};
		boost::asio::async_read(socket_, response_streambuf_, boost::asio::transfer_exactly(sizeof(RequestPackHeader)), read_pack_size_handler);
	}

	void ServerConnection::HandleReadPackHeader(const boost::system::error_code &error, size_t bytes_transferred)
	{
		if (error)
			throw boost::system::system_error(error);

		auto in_buffer = response_streambuf_.data();
		auto request_pack_header = *reinterpret_cast<const RequestPackHeader*>(&*boost::asio::buffers_begin(in_buffer));
		response_streambuf_.consume(bytes_transferred);

		auto shared_this = shared_from_this();
		auto read_pack_size_handler = [shared_this, request_pack_header](const boost::system::error_code &error, size_t bytes_transferred) {
			shared_this->HandleReadPack(error, bytes_transferred, request_pack_header);
		};
		boost::asio::async_read(socket_, response_streambuf_, boost::asio::transfer_exactly(request_pack_header.arg_length), read_pack_size_handler);
	}

	void ServerConnection::HandleReadPack(const boost::system::error_code &error, size_t bytes_transferred, const RequestPackHeader &request_pack_header)
	{
		if (error)
			throw boost::system::system_error(error);

		std::istream in_stream(&response_streambuf_);
        std::string buffer;
        buffer.resize(bytes_transferred);
        in_stream.read(&*buffer.begin(), bytes_transferred);

        auto res = GetRpcFunctions()[request_pack_header.func_name](buffer);

        ResponsePackHeader response_pack_header;
        response_pack_header.call_id = request_pack_header.call_id;
        response_pack_header.arg_length = res.size();

        std::string header_buffer;
        header_buffer.resize(sizeof(response_pack_header));
        memcpy(&*header_buffer.begin(), &response_pack_header, header_buffer.size());

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