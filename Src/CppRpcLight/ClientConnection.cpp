#include "ClientConnection.h"

namespace cpp_rpc_light
{
	ClientConnection::ClientConnection(boost::asio::io_service &io_service)
		: work_(io_service), resolver_(io_service), socket_(io_service), last_call_id_(0)
	{
		boost::asio::ip::tcp::resolver::query query("127.0.0.1", "12345");
		auto resolve_handler = [this](const boost::system::error_code &error, boost::asio::ip::tcp::resolver::iterator endpoint_it) {
			HandleResolve(error, endpoint_it);
		};
		resolver_.async_resolve(query, resolve_handler);
	}

	void ClientConnection::WaitForConnect()
	{
		auto connected_future = connected_promise_.get_future();
		connected_future.wait();
	}

	void ClientConnection::HandleResolve(const boost::system::error_code &error, boost::asio::ip::tcp::resolver::iterator endpoint_it)
	{
		if (error)
			throw boost::system::system_error(error);
		auto connect_handler = [this](const boost::system::error_code &error) {
			HandleConnect(error);
		};
		socket_.async_connect(*endpoint_it, connect_handler);
	}

	void ClientConnection::HandleConnect(const boost::system::error_code &error)
	{
		if (error)
			throw boost::system::system_error(error);
		connected_promise_.set_value();
	}
} // namespace cpp_rpc_light