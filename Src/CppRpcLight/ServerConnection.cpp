#include "ServerConnection.h"
#include <cstdint>

namespace cpp_rpc_light
{
	ServerConnection::Ptr ServerConnection::Create(ba::io_service &io_service)
	{
		return Ptr(new ServerConnection(io_service));
	}

	ServerConnection::ServerConnection(ba::io_service &io_service) 
		: socket_(io_service)
	{}

	void ServerConnection::Start()
	{
		auto shared_this = shared_from_this();
		auto read_pack_size_handler = [shared_this](const bs::error_code &error, size_t bytes_transferred) {
			shared_this->HandleReadPackSize(error, bytes_transferred);
		};
		ba::async_read(socket_, response_streambuf_, ba::transfer_exactly(sizeof(uint32_t)), read_pack_size_handler);
	}

	void ServerConnection::HandleReadPackSize(const bs::error_code &error, size_t bytes_transferred)
	{
		if (error)
			throw bs::system_error(error);

		auto in_buffer = response_streambuf_.data();
		auto pack_size = *reinterpret_cast<const uint32_t*>(&*ba::buffers_begin(in_buffer));
		response_streambuf_.consume(bytes_transferred);

		auto shared_this = shared_from_this();
		auto read_pack_size_handler = [shared_this](const bs::error_code &error, size_t bytes_transferred) {
			shared_this->HandleReadPack(error, bytes_transferred);
		};
		ba::async_read(socket_, response_streambuf_, ba::transfer_exactly(pack_size), read_pack_size_handler);
	}

	void ServerConnection::HandleReadPack(const bs::error_code &error, size_t bytes_transferred)
	{
		if (error)
			throw bs::system_error(error);

		auto in_buffer = response_streambuf_.data();
		std::string func_name(ba::buffers_begin(in_buffer), ba::buffers_begin(in_buffer) + bytes_transferred);
		response_streambuf_.consume(bytes_transferred);

		{
			std::string response("response");
			auto shared_buffer = std::make_shared<std::string>();
			shared_buffer->resize(sizeof(uint32_t));
			uint32_t buff_size = response.size();
			*shared_buffer->begin() = buff_size;
			*shared_buffer += response;
			auto buffer = ba::buffer(shared_buffer->data(), shared_buffer->size());

			auto write_handler = [this, shared_buffer](const boost::system::error_code &error, size_t bytes_transferred) {
				HandleWrite(error, bytes_transferred);
			};
			ba::async_write(socket_, buffer, write_handler);
		}
	}

	void ServerConnection::HandleWrite(const bs::error_code &error, size_t bytes_transferred)
	{
		if (error)
			throw bs::system_error(error);
	}
} // namespace cpp_rpc_light