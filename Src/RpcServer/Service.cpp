#include "Service.h"

RPC_DECLARE(sum, int, int a, int b) {
    return a + b;
}

RPC_DECLARE(echo, std::string, std::string str) {
    return "-->" + str;
}