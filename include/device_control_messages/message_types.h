#pragma once

#include <cstdint>

namespace hw::device_control_messages
{

enum class message_type : uint8_t
{
    measurement = 0,
    error       = 1,
    unknown     = 2,
};
}