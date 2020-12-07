#include <sstream>

#include <device_control_messages/error.h>

namespace hw::device_control_messages
{
std::string error::as_string() const
{
    std::stringstream ss;
    ss << header::as_string() << '\n';
    ss << "-----------------------------------\n";
    ss << "ERROR_TYPE=" << error_type_to_string(err_type) << '\n';
    ss << "-----------------------------------\n";
    return ss.str();
}
}