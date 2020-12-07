#pragma once

#include <string>

#include <device_control_messages/header.h>
#include <device_control_messages/message_types.h>

namespace hw::device_control_messages
{
class error : public header
{
public:
    error() = default;
    error(const std::string& device_name_)
        : header(device_name_, message_type::error)
    {}
};

}
