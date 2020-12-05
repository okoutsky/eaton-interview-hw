#pragma once

#include <string>
#include <vector>

#include <device_messages/header.h>
#include <device_messages/message_types.h>

namespace hw::device_messages
{
class measurement : public header
{
public:
    measurement(const std::string& device_name_)
        : header(device_name_, message_type::measurement)
    {}

public:
    std::vector<common::byte_type> serialize() const override;

    std::string as_string() const override
    {
        std::stringstream ss;
        ss << header::as_string() << "\n";
        ss << "-----------------------------------\n";
        ss << "TEMPERATURE SENSORS=(";
        for (size_t i = 0; i < _temperature_sensors.size(); i++)
        {
            ss << _temperature_sensors[i];
            if (i != _temperature_sensors.size() - 1)
                ss << ",";
        }
        ss << ")\n";
        ss << "FANS SPEED=(";
        for (size_t i = 0; i < _fans_speed.size(); i++)
        {
            ss << _fans_speed[i];
            if (i != _fans_speed.size() - 1)
                ss << ",";
        }
        ss << ")\n";

        // TODO: device specific data?

        return ss.str();
    }

private:
    size_t length() const override;

private:
    std::vector<uint16_t> _temperature_sensors;
    std::vector<uint8_t> _fans_speed;
    std::vector<common::byte_type> _device_specific;
};
}