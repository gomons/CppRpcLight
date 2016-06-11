#pragma once

#ifdef CPP_RPC_LIGHT_SERVER
#include <CppRpcLight/ServerMacro.h>
#else
#include <CppRpcLight/ClientMacro.h>
#endif

RPC_DEFINE(sum, int, int, int)

