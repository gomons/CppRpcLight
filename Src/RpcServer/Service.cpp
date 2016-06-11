#include "Service.h"

RPC_DECLARE(sum, int, int a, int b) {
    return a + b;
}