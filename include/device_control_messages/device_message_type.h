#pragma once

#include <variant>

#include <device_control_messages/error.h>
#include <device_control_messages/measurement.h>

namespace hw::device_control_messages
{
// TODO: ugly name
using device_message_type = std::variant<device_control_messages::measurement, device_control_messages::error>; // TODO: move from here

}