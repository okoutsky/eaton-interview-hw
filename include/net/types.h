#pragma once

#include <cstdint>
#include <string>

namespace hw::net
{

//! IP address type
using ip_address_t = std::string;
//! Transport layer port type
using port_t = uint16_t;

}