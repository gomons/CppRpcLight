#include "RpcNetworkProcessor.h"

namespace cpp_rpc_light
{
	RpcNetworkProcessor::RpcNetworkProcessor(ba::io_service &io_service)
		: io_service_(io_service)
	{}

	RpcNetworkProcessor::~RpcNetworkProcessor()
	{}

} // namespace cpp_rpc_light