#include "ServerConnection.h"
#include <cstdint>
#include "Pack.h"
#include "utils.h"
#include "serialize_tuple.h"

namespace
{
    template<typename F, typename Tuple, bool Enough, int TotalArgs, int... N>
    struct call_impl
    {
        auto static call(F f, Tuple&& t)
        {
            return call_impl<F, Tuple, TotalArgs == 1 + sizeof...(N),
                TotalArgs, N..., sizeof...(N)
            >::call(f, std::forward<Tuple>(t));
        }
    };

    template<typename F, typename Tuple, int TotalArgs, int... N>
    struct call_impl<F, Tuple, true, TotalArgs, N...>
    {
        auto static call(F f, Tuple&& t)
        {
            return f(std::get<N>(std::forward<Tuple>(t))...);
        }
    };

    template<typename F, typename Tuple>
    auto call(F f, Tuple&& t)
    {
        typedef typename std::decay<Tuple>::type type;
        return call_impl<F, Tuple, 0 == std::tuple_size<type>::value,
            std::tuple_size<type>::value
        >::call(f, std::forward<Tuple>(t));
    }
}

#define RPC_DEFINE(func_name, return_type, ...) \
    struct func_name##_descr { \
        typedef return_type ret_type; \
        typedef std::tuple<__VA_ARGS__> arg_type; \
    }; \
    return_type func_name(__VA_ARGS__);

#define RPC_DECLARE(func_name, return_type, ...) \
    struct func_name##adder { \
        func_name##adder() { \
            ServerConnection::rpc_functions[#func_name] = [] (const std::string &in) { \
                std::istringstream in_stream(in); \
                boost::archive::binary_iarchive in_archive(in_stream); \
                func_name##_descr::arg_type args; \
                in_archive >> args; \
                auto res = call(&func_name, args); \
                std::stringstream buffer_stream; \
                boost::archive::binary_oarchive out_archive(buffer_stream); \
                out_archive << res; \
                return buffer_stream.str(); \
            }; \
        } \
    } func_name##adder_; \
    return_type func_name(__VA_ARGS__)


namespace cpp_rpc_light
{
    std::map<std::string, std::function<std::string(const std::string&)>> ServerConnection::rpc_functions;

    RPC_DEFINE(sum, int, int, int)

    RPC_DECLARE(sum, int, int a, int b) {
        return a + b;
    }

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
			shared_this->HandleReadPackHeader(error, bytes_transferred);
		};
		ba::async_read(socket_, response_streambuf_, ba::transfer_exactly(sizeof(RequestPackHeader)), read_pack_size_handler);
	}

	void ServerConnection::HandleReadPackHeader(const bs::error_code &error, size_t bytes_transferred)
	{
		if (error)
			throw bs::system_error(error);

		auto in_buffer = response_streambuf_.data();
		auto request_pack_header = *reinterpret_cast<const RequestPackHeader*>(&*ba::buffers_begin(in_buffer));
		response_streambuf_.consume(bytes_transferred);

		auto shared_this = shared_from_this();
		auto read_pack_size_handler = [shared_this, request_pack_header](const bs::error_code &error, size_t bytes_transferred) {
			shared_this->HandleReadPack(error, bytes_transferred, request_pack_header);
		};
		ba::async_read(socket_, response_streambuf_, ba::transfer_exactly(request_pack_header.arg_length), read_pack_size_handler);
	}

	void ServerConnection::HandleReadPack(const bs::error_code &error, size_t bytes_transferred, const RequestPackHeader &request_pack_header)
	{
		if (error)
			throw bs::system_error(error);

		std::istream in_stream(&response_streambuf_);
        std::string buffer;
        buffer.resize(bytes_transferred);
        in_stream.read(&*buffer.begin(), bytes_transferred);

        auto res = rpc_functions[request_pack_header.func_name](buffer);

        ResponsePackHeader response_pack_header;
        response_pack_header.call_id = request_pack_header.call_id;
        response_pack_header.arg_length = res.size();

        std::string header_buffer;
        header_buffer.resize(sizeof(response_pack_header));
        memcpy(&*header_buffer.begin(), &response_pack_header, header_buffer.size());

        auto buffer_ptr = std::make_shared<std::string>();
        *buffer_ptr += header_buffer + res;
        auto out_buffer = ba::buffer(buffer_ptr->data(), buffer_ptr->size());

        auto shared_this = shared_from_this();
		auto write_handler = [shared_this, buffer_ptr](const boost::system::error_code &error, size_t bytes_transferred) {
            shared_this->HandleWrite(error, bytes_transferred);
		};
		ba::async_write(socket_, out_buffer, write_handler);
	}

	void ServerConnection::HandleWrite(const bs::error_code &error, size_t bytes_transferred)
	{
		if (error)
			throw bs::system_error(error);

        Start();
	}
} // namespace cpp_rpc_light