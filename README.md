# CppRpcLight

Code example for [CoreHard summer conference 2016](https://corehard.by/2016/06/20/corehard-summer-2016-generic-programming-in-c/).

## How to build example
1. Install Visual Studio 2015
2. Download [prebuilt boost](https://1drv.ms/f/s!AhSSCCTw8v3yqz0w7lmvpcowyWR6)
3. Extract boost archive to ThirdParty directory (`ThirdParty\boost\1.60.0\`)
4. Open `Src/CppRpcLight.sln` solution in Visual Studio 2015
5. Build `RpcServer` and `RpcClient` applications
6. Run `RpcServer` than run `RpcServer`

## Code example

Define service interface in server application (Service.h):
```c++
RPC_DEFINE(sum, int, int, int);
RPC_DEFINE(echo, std::string, std::string);
```

Declare service implementation in server application (Service.cpp):
```c++
#include "Service.h"

RPC_DECLARE(sum, int, int a, int b) {
    return a + b;
}

RPC_DECLARE(echo, std::string, std::string str) {
    return "-->" + str;
}
```

Add Service.h file to client application and call remove functions:
```c++
// Blocked call
std::string echo_result = echo(client_connection, std::string("Ping!"));
std::cout << echo_result << std::endl;

// Async call
std::future<std::string> echo_future = echo_async(client_connection, std::string("Ping!"));
std::cout << echo_future.get() << std::endl;
```

## Contact info
If you have any questions, feel free to contact me at gomons@gmail.com
