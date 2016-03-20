#pragma once

#include <algorithm>
#include <future>
#include <sstream>
#include <map>
#include <sstream>
#include <boost/asio.hpp>
#include "aliases.h"
#include "Pack.h"
#include "utils.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "PackHeader.h"
#include "serialize_tuple.h"

namespace cpp_rpc_light
{
	class ClientConnection
	{
	public:
		ClientConnection(ba::io_service &io_service);

		void WaitForConnect();

        template<typename ReturnType, typename... Args>
		std::future<typename ReturnType> ExecuteFunction(const std::string &func_name, Args... args);

	private:
		void HandleResolve(const bs::error_code &error, ba_tcp::resolver::iterator endpoint_it);
		void HandleConnect(const bs::error_code &error);

		template<typename T> void HandleWrite(
			const bs::error_code &error, 
			size_t bytes_transferred,
			std::shared_ptr<std::promise<T>> shared_promise);
		template<typename T> void HandleReadPackHeader(
			const bs::error_code &error, 
			size_t bytes_transferred,
			std::shared_ptr<std::promise<T>> shared_promise);
		template<typename T> void HandleReadPack(
			const bs::error_code &error, 
			size_t bytes_transferred,
			std::shared_ptr<std::promise<T>> shared_promise,
            const ResponsePackHeader &response_pack_header);

		ba::io_service::work work_;
		ba_tcp::resolver resolver_;
		ba_tcp::socket socket_;
		ba::streambuf response_streambuf_;
		std::promise<void> connected_promise_;

        uint32_t last_call_id_;
        std::map<uint32_t, std::function<void(const std::string&)>> waited_results_;
	};

    template<typename ReturnType, typename... Args>
    std::future<typename ReturnType> ClientConnection::ExecuteFunction(const std::string &func_name, Args... args)
    {
        auto promise_ptr = std::make_shared<std::promise<ReturnType>>();
        waited_results_[++last_call_id_] = [promise_ptr](const std::string &res) {
            std::istringstream in_stream(res);
            boost::archive::binary_iarchive in_archive(in_stream);
            ReturnType ret;
            in_archive >> ret;
            promise_ptr->set_value(ret);
        };

        std::tuple<Args...> args(args...);
        std::stringstream args_buffer_stream;
        boost::archive::binary_oarchive out_archive(args_buffer_stream);
        out_archive << args;
        args_buffer_stream.seekg(0, std::ios::end);
        uint32_t args_buffer_stream_size = static_cast<uint32_t>(args_buffer_stream.tellg());
        args_buffer_stream.seekg(0, std::ios::beg);

        RequestPackHeader header = {};
        header.arg_length = args_buffer_stream_size;
        header.call_id = last_call_id_;
        strcpy(header.func_name, func_name.c_str());
        std::string header_buffer;
        header_buffer.resize(sizeof(header));
        memcpy(&*header_buffer.begin(), &header, header_buffer.size());
        

        auto buffer_ptr = std::make_shared<std::string>();
        *buffer_ptr += header_buffer + args_buffer_stream.str();
        auto buffer = ba::buffer(buffer_ptr->data(), buffer_ptr->size());

        auto write_handler = [this, buffer_ptr, promise_ptr](const boost::system::error_code &error, size_t bytes_transferred) {
            HandleWrite(error, bytes_transferred, promise_ptr);
        };
        ba::async_write(socket_, buffer, write_handler);

        return promise_ptr->get_future();
    }

	template<typename T>
	void ClientConnection::HandleWrite(
		const bs::error_code &error,
		size_t bytes_transferred,
		std::shared_ptr<std::promise<T>> shared_promise)
	{
		if (error)
			throw bs::system_error(error);

		auto read_pack_size_handler = [this, shared_promise](const bs::error_code &error, size_t bytes_transferred) {
            HandleReadPackHeader(error, bytes_transferred, shared_promise);
		};
		ba::async_read(socket_, response_streambuf_, ba::transfer_exactly(sizeof(ResponsePackHeader)), read_pack_size_handler);
	}

	template<typename T>
	void ClientConnection::HandleReadPackHeader(
		const bs::error_code &error,
		size_t bytes_transferred,
		std::shared_ptr<std::promise<T>> shared_promise)
	{
		auto in_buffer = response_streambuf_.data();
		auto response_pack_header = *reinterpret_cast<const ResponsePackHeader*>(&*ba::buffers_begin(in_buffer));
		response_streambuf_.consume(bytes_transferred);
		auto read_pack_size_handler = [this, shared_promise, response_pack_header](const bs::error_code &error, size_t bytes_transferred) {
			HandleReadPack(error, bytes_transferred, shared_promise, response_pack_header);
		};
		ba::async_read(socket_, response_streambuf_, ba::transfer_exactly(response_pack_header.arg_length), read_pack_size_handler);
	}

	template<typename T>
	void ClientConnection::HandleReadPack(
		const bs::error_code &error,
		size_t bytes_transferred,
		std::shared_ptr<std::promise<T>> promise_ptr,
        const ResponsePackHeader &response_pack_header)
	{
		if (error)
			throw bs::system_error(error);

        std::istream in_stream(&response_streambuf_);
        std::string buffer;
        buffer.resize(bytes_transferred);
        in_stream.read(&*buffer.begin(), bytes_transferred);
        waited_results_[response_pack_header.call_id](buffer);
	}
} // namespace cpp_rpc_light