#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <sstream>
#include <string>

#include <common/config.h>
#include <common/types.h>
#include <device_protocol/message_types.h>

namespace hw::device_control_messages
{
class header
{
public:
    header(const std::string& device_name_, message_type msg_type_)
        : _device_name(device_name_)
        , _msg_type(msg_type_)
    {
        if (device_name_.size() > common::config::max_device_name_length)
            throw std::runtime_error(std::string{"Device name longer than maximal allowed length ("} + std::to_string(common::config::max_device_name_length) +
                                     std::string{")"});
    }

    virtual ~header() = default;

    virtual std::vector<common::byte_t> serialize() const
    {
        // TODO: nicer copuing
        std::vector<common::byte_t> result(length());
        std::copy(_device_name.begin(), _device_name.end(), result.begin());

        auto* data        = result.data() + common::config::max_device_name_length;
        auto msg_type_num = static_cast<uint8_t>(_msg_type);
        std::memcpy(data, &msg_type_num, sizeof(msg_type_num));
        data += sizeof(msg_type_num);

        auto len = length();
        std::memcpy(data, &len, sizeof(len));

        return result;
    }

    virtual std::string as_string() const
    {
        std::stringstream ss;
        ss << "Device message HEADER\n";
        ss << "-----------------------------------\n";
        ss << "DEVICE NAME=" << _device_name << "\n";
        ss << "MSG TYPE="
           << "AAA" /*TODO: boost enums?*/ << "\n";
        ss << "LENGTH=" << _length << "\n";

        return ss.str();
    }

    virtual std::tuple<size_t> deserialize(std::vector<common::byte_t> data_);

    const std::string& get_device_name() const { return _device_name; }

    message_type get_message_type() const { return _msg_type; }

protected:
    virtual size_t length() const { return sizeof(common::byte_t) * _device_name.size() + sizeof(_msg_type) + sizeof(_length); }

private:
    std::string _device_name;
    message_type _msg_type;
    uint32_t _length;
};
}