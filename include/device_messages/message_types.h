#pragma once

#include <cstdint>

namespace hw::device_messages
{

enum class message_type : uint8_t
{
    measurement = 0,
    error       = 1,
};
}