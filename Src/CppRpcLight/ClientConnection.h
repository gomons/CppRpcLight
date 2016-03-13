#pragma once

#include <future>
#include <boost/asio.hpp>
#include "aliases.h"

namespace cpp_rpc_light
{
	class ClientConnection
	{
	public:
		ClientConnection(ba::io_service &io_service);

		void WaitForConnect();

		template<typename T>
		std::future<T> ExecuteFunction(const std::string &func_name);

	private:
		void HandleResolve(const bs::error_code &error, ba_tcp::resolver::iterator endpoint_it);
		void HandleConnect(const bs::error_code &error);

		template<typename T> void HandleWrite(
			const bs::error_code &error, 
			size_t bytes_transferred,
			std::shared_ptr<std::promise<T>> shared_promise);
		template<typename T> void HandleReadPackSize(
			const bs::error_code &error, 
			size_t bytes_transferred,
			std::shared_ptr<std::promise<T>> shared_promise);
		template<typename T> void HandleReadPack(
			const bs::error_code &error, 
			size_t bytes_transferred,
			std::shared_ptr<std::promise<T>> shared_promise);

		ba::io_service::work work_;
		ba_tcp::resolver resolver_;
		ba_tcp::socket socket_;
		ba::streambuf response_streambuf_;
		std::promise<void> connected_promise_;
	};

	template<typename T>
	void ClientConnection::HandleWrite(
		const bs::error_code &error,
		size_t bytes_transferred,
		std::shared_ptr<std::promise<T>> shared_promise)
	{
		if (error)
			throw bs::system_error(error);

		auto read_pack_size_handler = [this, shared_promise](const bs::error_code &error, size_t bytes_transferred) {
			HandleReadPackSize(error, bytes_transferred, shared_promise);
		};
		ba::async_read(socket_, response_streambuf_, ba::transfer_exactly(sizeof(uint32_t)), read_pack_size_handler);
	}

	template<typename T>
	void ClientConnection::HandleReadPackSize(
		const bs::error_code &error,
		size_t bytes_transferred,
		std::shared_ptr<std::promise<T>> shared_promise)
	{
		auto in_buffer = response_streambuf_.data();
		auto pack_size = *reinterpret_cast<const uint32_t*>(&*ba::buffers_begin(in_buffer));
		response_streambuf_.consume(bytes_transferred);

		auto read_pack_size_handler = [this, shared_promise](const bs::error_code &error, size_t bytes_transferred) {
			HandleReadPack(error, bytes_transferred, shared_promise);
		};
		ba::async_read(socket_, response_streambuf_, ba::transfer_exactly(pack_size), read_pack_size_handler);
	}

	template<typename T>
	inline void SetValue(const std::string &data, std::shared_ptr<std::promise<T>> &shared_promise)
	{
		T value(data);
		shared_promise->set_value(value);
	}

	template<>
	inline void SetValue<void>(const std::string &data, std::shared_ptr<std::promise<void>> &shared_promise)
	{
		shared_promise->set_value();
	}

	template<typename T>
	void ClientConnection::HandleReadPack(
		const bs::error_code &error,
		size_t bytes_transferred,
		std::shared_ptr<std::promise<T>> shared_promise)
	{
		if (error)
			throw bs::system_error(error);

		auto in_buffer = response_streambuf_.data();
		std::string func_name(ba::buffers_begin(in_buffer), ba::buffers_begin(in_buffer) + bytes_transferred);
		response_streambuf_.consume(bytes_transferred);

		SetValue(func_name, shared_promise);
	}

	template<typename T>
	std::future<T> ClientConnection::ExecuteFunction(const std::string &func_name)
	{
		auto shared_promise = std::make_shared<std::promise<T>>();

		auto shared_buffer = std::make_shared<std::string>();
		shared_buffer->resize(sizeof(uint32_t));
		uint32_t buff_size = func_name.size();
		*shared_buffer->begin() = buff_size;
		*shared_buffer += func_name;
		auto buffer = ba::buffer(shared_buffer->data(), shared_buffer->size());

		auto write_handler = [this, shared_buffer, shared_promise](const boost::system::error_code &error, size_t bytes_transferred) {
			HandleWrite(error, bytes_transferred, shared_promise);
		};
		ba::async_write(socket_, buffer, write_handler);

		return shared_promise->get_future();
	}
} // namespace cpp_rpc_light