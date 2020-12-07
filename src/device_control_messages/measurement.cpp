#include <sstream>

#include <device_control_messages/measurement.h>

namespace hw::device_control_messages
{
std::string measurement::as_string() const
{
    std::stringstream ss;
    ss << header::as_string() << "\n";
    ss << "-----------------------------------\n";
    ss << "TEMPERATURE SENSORS=(";
    for (size_t i = 0; i < temperature_sensors.size(); i++)
    {
        ss << temperature_sensors[i];
        if (i != temperature_sensors.size() - 1)
            ss << ",";
    }
    ss << ")\n";
    ss << "FANS SPEED=(";
    for (size_t i = 0; i < fans_speed.size(); i++)
    {
        ss << fans_speed[i];
        if (i != fans_speed.size() - 1)
            ss << ",";
    }
    ss << ")\n";

    // TODO: device specific data?

    return ss.str();
}
}