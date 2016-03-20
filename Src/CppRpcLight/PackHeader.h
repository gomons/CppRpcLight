#pragma once

#include <cstdint>

struct RequestPackHeader 
{
    char func_name[256];
    uint32_t call_id;
    uint32_t arg_length;
};

struct ResponsePackHeader
{
    uint32_t call_id;
    uint32_t arg_length;
};