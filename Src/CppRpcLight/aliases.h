#pragma once

namespace boost 
{ 
	namespace asio 
	{
		namespace ip {}
	}  
	namespace system {}
}

namespace cpp_rpc_light
{
	namespace ba = boost::asio;
	namespace ba_ip = boost::asio::ip;
	namespace bs = boost::system;

	typedef ba_ip::tcp ba_tcp;
} //namespace cpp_rpc_light