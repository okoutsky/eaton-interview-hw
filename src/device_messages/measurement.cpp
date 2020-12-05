#include <device_messages/measurement.h>

namespace hw::device_messages
{
std::vector<common::byte_type> measurement::serialize() const
{
    auto result = header::serialize();
    result.resize(length());
    auto* data = result.data() + header::length();

    uint32_t num_temp_sensors = _temperature_sensors.size();
    std::memcpy(data, &num_temp_sensors, sizeof(num_temp_sensors));
    data += sizeof(num_temp_sensors);

    for (auto sensor : _temperature_sensors)
    {
        std::memcpy(data, &sensor, sizeof(sensor));
        data += sizeof(sensor);
    }

    uint32_t num_fans_speeds = _fans_speed.size();
    std::memcpy(data, &num_fans_speeds, sizeof(num_fans_speeds));
    data += sizeof(num_fans_speeds);

    for (auto fan_speed : _fans_speed)
    {
        std::memcpy(data, &fan_speed, sizeof(fan_speed));
        data += sizeof(fan_speed);
    }

    uint32_t num_specific_bytes = _device_specific.size();
    std::memcpy(&data, &num_specific_bytes, sizeof(num_specific_bytes));
    data += sizeof(num_specific_bytes);

    std::memcpy(&data, _device_specific.data(), _device_specific.size());

    return result;
}

size_t measurement::length() const
{
    return header::length() + sizeof(uint16_t) * _temperature_sensors.size() + sizeof(uint8_t) * _fans_speed.size() +
           sizeof(common::byte_type) * _device_specific.size();
}
}