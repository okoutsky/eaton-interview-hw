#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <device_control_messages/messages.h>

namespace hw
{

/**
 * @brief Class storing device control messages and providing methods for accessing them.
 */
class device_messages_storage
{
public:
    device_messages_storage() = default;

    /**
     * @brief Store new message
     *
     * @param message_ message
     */
    void new_message(device_control_messages::device_message_type message_)
    {
        std::scoped_lock lock(_internal_access_mtx);
        new_message_impl(std::move(message_));
    }

    /**
     * @brief Get messages from given device
     *
     * @param device_name_ device to get messages from
     * @return vector of device messages
     */
    std::vector<device_control_messages::device_message_type> get_device_messages(const std::string& device_name_)
    {
        std::scoped_lock lock(_internal_access_mtx);
        return get_device_messages_impl(device_name_);
    }

    /**
     * @brief Get messages of provided type received from given device
     *
     * @tparam ReuqestedMessageType Type of messages to retrieve
     * @param device_name_ device to get messages from
     * @return vector of @ref RequestMessagesType messages
     */
    template <class ReuqestedMessageType>
    std::vector<ReuqestedMessageType> get_device_messages_of_type(const std::string& device_name_)
    {
        std::scoped_lock lock(_internal_access_mtx);
        return get_device_messages_of_type_impl<ReuqestedMessageType>(device_name_);
    }

    /**
     * @brief Get all devivec for whom some messages have been received
     *
     * @return vector of device names
     */
    std::vector<std::string> get_devices()
    {
        std::scoped_lock lock(_internal_access_mtx);
        return get_devices_impl();
    }

private:
    // Thread-unsafe implementation of new_message(...) public method
    void new_message_impl(const device_control_messages::device_message_type& message_);

    // Thread-unsafe implementation of get_device_messages(...) public method
    std::vector<device_control_messages::device_message_type> get_device_messages_impl(const std::string& device_);

    // Thread-unsafe implementation of get_device_messages_of_type_impl<...>(...) public method
    template <class ReuqestedMessageType>
    std::vector<ReuqestedMessageType> get_device_messages_of_type_impl(const std::string& name_)
    {
        auto messages = get_device_messages_impl(name_);
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

    // Thread-unsafe implementation of get_devices_impl(...) public method
    std::vector<std::string> get_devices_impl();

    // Method for logging purposes
    const std::string me() { return "[device_messages_storage] "; }

private:
    std::unordered_map<std::string, std::vector<device_control_messages::device_message_type>> _messages_received;
    std::mutex _internal_access_mtx;
};
}