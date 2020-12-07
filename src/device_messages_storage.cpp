#include <device_messages_storage.h>

namespace hw
{
void device_messages_storage::new_message_impl(const device_control_messages::device_message_type& message_)
{
    auto device_name_visitor = [](auto msg_) { return msg_.device_name; };
    auto name                = std::visit(device_name_visitor, message_);

    auto iter = _messages_received.find(name);
    if (iter == _messages_received.end())
        _messages_received[name] = {std::move(message_)};
    else
        _messages_received[name].push_back(std::move(message_));
}

std::vector<device_control_messages::device_message_type> device_messages_storage::get_device_messages_impl(const std::string& device_)
{
    auto iter = _messages_received.find(device_);
    if (iter != _messages_received.end())
        return iter->second;
    else
        return {};
}

std::vector<std::string> device_messages_storage::get_devices_impl()
{
    std::vector<std::string> devices;
    for (const auto& map_pair : _messages_received)
    {
        devices.push_back(map_pair.first);
    }
    return devices;
}

}