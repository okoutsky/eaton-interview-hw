#pragma once

#include <cstdint>
#include <string>

namespace hw::device_control_messages
{

enum class message_type : uint8_t
{
    measurement = 0,
    error       = 1,
    unknown     = 2,
};

inline std::string message_type_to_string(message_type msg_type_)
{
    if (msg_type_ == message_type::measurement)
        return "measurement";
    if (msg_type_ == message_type::error)
        return "error";

    return "unknown";
}
}