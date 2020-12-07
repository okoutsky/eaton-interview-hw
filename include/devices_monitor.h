#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>

#include <device_control_messages/messages.h>

namespace hw
{

class devices_monitor
{
public:
    devices_monitor() = default;

    void new_message(device_control_messages::device_message_type message_) { new_message_impl(std::move(message_)); }

    std::vector<device_control_messages::device_message_type> get_device_messages(const std::string& device_)
    {
        auto iter = _messages_received.find(device_);
        if (iter != _messages_received.end())
            return iter->second;
        else
            return {};
    }

    template <class ReuqestedMessageType>
    std::vector<ReuqestedMessageType> get_device_messages_of_type(const std::string& name_)
    {
        auto messages = get_device_messages(name_);
        std::vector<ReuqestedMessageType> result;
        for (const auto& msg : messages)
        {
            try
            {
                result.push_back(std::get<ReuqestedMessageType>(msg));
            }
            catch (const std::bad_variant_access&)
            {
                // Message of different type
            }
        }
        return result;
    }

    std::vector<std::string> get_devices()
    {
        std::vector<std::string> devices;
        for (const auto& map_pair : _messages_received)
        {
            devices.push_back(map_pair.first);
        }
        return devices;
    }

private:
    void new_message_impl(const device_control_messages::device_message_type& message_)
    {
        auto device_name_visitor = [](auto msg_) { return msg_.device_name; };
        auto name                = std::visit(device_name_visitor, message_);

        auto iter = _messages_received.find(name);
        if (iter == _messages_received.end())
            _messages_received[name] = {std::move(message_)};
        else
            _messages_received[name].push_back(std::move(message_));
    }

private:
    const std::string me() { return "[devices_monitor] "; }
    std::unordered_map<std::string, std::vector<device_control_messages::device_message_type>> _messages_received;
};
}