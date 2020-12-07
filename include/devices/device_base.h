#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <common/handler_holder.h>
#include <common/types.h>
#include <device_control_messages/measurement.h>

namespace hw::devices
{

class device_base
{
public:
    device_base(const std::string& name_)
        : _name(name_)
    {}

    virtual ~device_base() = default;

public:
    common::handler_holder<void(device_control_messages::measurement)> on_measurement;

protected:
    const std::string _name;
};
}