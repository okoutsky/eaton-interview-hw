#include <sstream>

#include <device_control_messages/header.h>

namespace hw::device_control_messages
{
std::string header::as_string() const
{
    std::stringstream ss;
    ss << "-----------------------------------\n";
    ss << "Device message HEADER\n";
    ss << "-----------------------------------\n";
    ss << "DEVICE NAME=" << device_name << '\n';
    ss << "MSG TYPE=" << message_type_to_string(message_type) << '\n';
    return ss.str();
}
}