#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <string>

#include <common/types.h>
#include <device_control_messages/message_types.h>

namespace hw::device_control_messages
{
class header
{
public:
    header() = default;
    header(const std::string& device_name_, message_type msg_type_)
        : device_name(device_name_)
        , message_type(msg_type_)
    {}

    virtual ~header() = default;

    virtual std::string as_string() const;

public:
    std::string device_name{};
    message_type message_type{message_type::unknown};
};
}