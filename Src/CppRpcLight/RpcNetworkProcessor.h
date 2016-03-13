#pragma once

#include <boost/asio.hpp>
#include "aliases.h"

namespace cpp_rpc_light 
{
	class RpcNetworkProcessor 
	{
	public:
		RpcNetworkProcessor(ba::io_service &io_service);
		~RpcNetworkProcessor();


	private:
		ba::io_service &io_service_;
	};

} // namespace cpp_rpc_light