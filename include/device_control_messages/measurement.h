#pragma once

#include <string>
#include <vector>

#include <device_control_messages/header.h>
#include <device_control_messages/message_types.h>

namespace hw::device_control_messages
{
class measurement : public header
{
public:
    static constexpr uint16_t error_temperature = 0xffff;
    static constexpr uint8_t error_fan_speed    = 0xff;

public:
    measurement() = default;
    measurement(const std::string& device_name_)
        : header(device_name_, message_type::measurement)
    {}

public:
    std::string as_string() const override;

public:
    std::vector<uint16_t> temperature_sensors;
    std::vector<uint8_t> fans_speed;
};
}