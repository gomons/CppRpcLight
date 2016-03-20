#include "ClientConnection.h"
#include <cstdint>

namespace cpp_rpc_light
{
	ClientConnection::ClientConnection(ba::io_service &io_service)
		: work_(io_service), resolver_(io_service), socket_(io_service)
	{
        last_call_id_ = 0;

		ba_tcp::resolver::query query("127.0.0.1", "12345");
		auto resolve_handler = [this](const bs::error_code &error, ba_tcp::resolver::iterator endpoint_it) {
			HandleResolve(error, endpoint_it);
		};
		resolver_.async_resolve(query, resolve_handler);
	}

	void ClientConnection::WaitForConnect()
	{
		auto connected_future = connected_promise_.get_future();
		connected_future.wait();
	}

	void ClientConnection::HandleResolve(const bs::error_code &error, ba_tcp::resolver::iterator endpoint_it)
	{
		if (error)
			throw bs::system_error(error);

		ba_tcp::endpoint endpoint = *endpoint_it;
		auto connect_handler = [this](const bs::error_code &error) {
			HandleConnect(error);
		};
		socket_.async_connect(endpoint, connect_handler);
	}

	void ClientConnection::HandleConnect(const bs::error_code &error)
	{
		if (error)
			throw bs::system_error(error);

		connected_promise_.set_value();
	}
} // namespace cpp_rpc_light