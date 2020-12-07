#pragma once

#include <cstddef>
#include <string>

#include <common/handler_holder.h>
#include <common/types.h>
#include <device_control_messages/messages.h>

namespace hw::devices
{

/**
 * @brief Base class for all devices
 * Poviding basic device interface.
 */
class device_base
{
public:
    /**
     * @brief Constructor
     *
     * @param name_ Name of the device
     */
    device_base(const std::string& name_)
        : _name(name_)
    {}

    virtual ~device_base() = default;

public:
    //! Callback triggered when device control message is produced. Callback parameter: device control message produced.
    common::handler_holder<void(device_control_messages::device_message_type)> on_message;

protected:
    const std::string _name; ///< Name of the device
};
}