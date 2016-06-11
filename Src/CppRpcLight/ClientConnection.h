#pragma once

#include <cstdint>
#include <algorithm>
#include <atomic>
#include <future>
#include <map>
#include <mutex>
#include <sstream>
#include <sstream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/asio.hpp>
#include "PackHeader.h"
#include "TupleSerialization.h"

namespace cpp_rpc_light
{
    // Class is thread safe
    class ClientConnection
    {
    public:
        ClientConnection(boost::asio::io_service &io_service);

        void WaitForConnect();

        template<typename ReturnType, typename... Args> 
        std::future<typename ReturnType> ExecuteFunction(const std::string &func_name, Args... args);

    private:
        void HandleResolve(
            const boost::system::error_code &error, 
            boost::asio::ip::tcp::resolver::iterator endpoint_it);
        void HandleConnect(const boost::system::error_code &error);

        template<typename T> 
        void HandleWrite(
            const boost::system::error_code &error, 
            size_t bytes_transferred, 
            std::shared_ptr<std::promise<T>> shared_promise);
        
        template<typename T> void HandleReadPackHeader(
            const boost::system::error_code &error, 
            size_t bytes_transferred,
            std::shared_ptr<boost::asio::streambuf> response_shared_streambuf,
            std::shared_ptr<std::promise<T>> shared_promise);

        template<typename T> void HandleReadPack(
            const boost::system::error_code &error, 
            size_t bytes_transferred,
            std::shared_ptr<boost::asio::streambuf> response_shared_streambuf,
            const ResponsePackHeader &response_pack_header,
            std::shared_ptr<std::promise<T>> shared_promise);

        boost::asio::io_service::work work_;
        boost::asio::ip::tcp::resolver resolver_;
        boost::asio::ip::tcp::socket socket_;
        std::promise<void> connected_promise_;

        std::mutex waited_results_mutex;
        std::atomic_int last_call_id_;
        std::map<uint32_t, std::function<void(const std::string&)>> waited_results_;
    };

    template<typename ReturnType, typename... Args>
    std::future<typename ReturnType> ClientConnection::ExecuteFunction(
        const std::string &func_name, Args... args)
    {
        int current_call_id = ++last_call_id_;

        // Serialize arguments
        std::tuple<Args...> args(args...);
        std::stringstream args_buffer_stream;
        boost::archive::binary_oarchive out_archive(args_buffer_stream);
        out_archive << args;
        auto args_buffer = args_buffer_stream.str();
        args_buffer_stream.seekg(0, std::ios::end);
        auto args_buffer_stream_size = args_buffer_stream.tellg();

        // Serialize package header
        RequestPackHeader header = {};
        header.arg_length = static_cast<uint32_t>(args_buffer_stream_size);
        header.call_id = current_call_id;
        func_name.copy(header.func_name, func_name.size());
        std::string header_buffer(reinterpret_cast<const char*>(&header), sizeof(header));
        
        // Register function call
        auto shared_promise = std::make_shared<std::promise<ReturnType>>();
        {
            std::lock_guard<std::mutex> lock(waited_results_mutex);
            waited_results_[current_call_id] = [shared_promise](const std::string &res) {
                std::istringstream in_stream(res);
                boost::archive::binary_iarchive in_archive(in_stream);
                ReturnType ret;
                in_archive >> ret;
                shared_promise->set_value(ret);
            };
        }

        // Call remote function
        auto shared_buffer = std::make_shared<std::string>(header_buffer + args_buffer);
        auto buffer = boost::asio::buffer(shared_buffer->data(), shared_buffer->size());
        auto write_handler = [this, shared_buffer, shared_promise](const boost::system::error_code &error, size_t bytes_transferred) {
            HandleWrite(error, bytes_transferred, shared_promise);
        };
        boost::asio::async_write(socket_, buffer, write_handler);

        return shared_promise->get_future();
    }

    template<typename T> 
    void ClientConnection::HandleWrite(
        const boost::system::error_code &error, 
        size_t bytes_transferred, 
        std::shared_ptr<std::promise<T>> shared_promise)
    {
        if (error)
            throw boost::system::system_error(error);
        auto response_shared_streambuf = std::make_shared<boost::asio::streambuf>();
        auto read_pack_size_handler = [this, response_shared_streambuf, shared_promise](const boost::system::error_code &error, size_t bytes_transferred) {
            HandleReadPackHeader(error, bytes_transferred, response_shared_streambuf, shared_promise);
        };
        boost::asio::async_read(socket_, *response_shared_streambuf, boost::asio::transfer_exactly(sizeof(ResponsePackHeader)), read_pack_size_handler);
    }

    template<typename T> 
    void ClientConnection::HandleReadPackHeader(
        const boost::system::error_code &error, 
        size_t bytes_transferred, 
        std::shared_ptr<boost::asio::streambuf> response_shared_streambuf, 
        std::shared_ptr<std::promise<T>> shared_promise)
    {
        if (error)
            throw boost::system::system_error(error);
        auto in_buffer = response_shared_streambuf->data();
        auto response_pack_header = *reinterpret_cast<const ResponsePackHeader*>(&*boost::asio::buffers_begin(in_buffer));
        response_shared_streambuf->consume(bytes_transferred);
        auto read_pack_size_handler = [this, response_shared_streambuf, response_pack_header, shared_promise](const boost::system::error_code &error, size_t bytes_transferred) {
            HandleReadPack(error, bytes_transferred, response_shared_streambuf, response_pack_header, shared_promise);
        };
        boost::asio::async_read(socket_, *response_shared_streambuf, boost::asio::transfer_exactly(response_pack_header.arg_length), read_pack_size_handler);
    }

    template<typename T> 
    void ClientConnection::HandleReadPack(
        const boost::system::error_code &error, 
        size_t bytes_transferred, 
        std::shared_ptr<boost::asio::streambuf> response_shared_streambuf, 
        const ResponsePackHeader &response_pack_header,
        std::shared_ptr<std::promise<T>> promise_ptr)
    {
        if (error)
            throw boost::system::system_error(error);
        std::istream in_stream(response_shared_streambuf.get());
        std::string buffer(bytes_transferred, '\0');
        in_stream.read(&*buffer.begin(), bytes_transferred);
        {
            std::lock_guard<std::mutex> lock(waited_results_mutex);
            waited_results_[response_pack_header.call_id](buffer);
            waited_results_.erase(response_pack_header.call_id);
        }
    }
} // namespace cpp_rpc_light