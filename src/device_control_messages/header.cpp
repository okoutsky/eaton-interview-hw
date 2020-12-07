#include <sstream>

#include <device_control_messages/header.h>

namespace hw::device_control_messages
{
std::string header::as_string() const
{
    std::stringstream ss;
    ss << "Device message HEADER\n";
    ss << "-----------------------------------\n";
    ss << "DEVICE NAME=" << device_name << "\n";
    ss << "MSG TYPE="
       << "AAA" /*TODO: boost enums?*/ << "\n";

    return ss.str();
}
}