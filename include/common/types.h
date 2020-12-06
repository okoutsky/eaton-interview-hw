#pragma once

#include <cstddef>
#include <string>

namespace hw::common
{
using error_code = size_t;

using byte_t = uint8_t;

using ip_address_t = std::string;
using port_t       = uint16_t;

}