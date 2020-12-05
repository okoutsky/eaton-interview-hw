#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <common/handler_holder.h>
#include <common/types.h>

namespace hw
{

class device_base
{
public:
    using on_measurement_handler_t = void(std::string, std::vector<common::byte_type>);
    using on_error_handler_t       = void(common::error_code);

public:
    device_base(const std::string& name_)
        : _name(name_)
    {}

    virtual ~device_base() = default;

public:
    common::handler_holder<on_measurement_handler_t> on_measurement;
    common::handler_holder<on_error_handler_t> on_error;

private:
    const std::string _name;
};
}