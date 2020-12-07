#pragma once

#include <cstddef>
#include <string>

#include <common/types.h>
#include <device_control_messages/message_types.h>

namespace hw::device_control_messages
{
/** @brief Message header common to all device control messages */
class header
{
public:
    header() = default;

    /**
     * @brief Constructor
     *
     * @param device_name_ Device name
     * @param msg_type_ Message type
     */
    header(const std::string& device_name_, message_type msg_type_)
        : device_name(device_name_)
        , message_type(msg_type_)
    {}

    virtual ~header() = default;

    /**
     * @brief Print message in human readable format
     *
     * @return Readable message
     */
    virtual std::string as_string() const;

public:
    std::string device_name{};                        ///< Device name
    message_type message_type{message_type::unknown}; ///< Message type
};
}