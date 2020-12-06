#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>

#include <common/asynchronous.h>
#include <device_protocol/measurement.h>

namespace hw
{
class devices_monitor : public common::asynchronous<devices_monitor>
{
public:
    devices_monitor(boost::asio::io_context& ioc_)
        : common::asynchronous<devices_monitor>(ioc_)
    {}

    void new_measurement(std::shared_ptr<device_protocol::measurement> measurement_) // Is shared ptr necessary?
    {
        if (!measurement_)
        {
            std::cerr << me() << "Invalid message received. Dropping.";
            return;
        }

        std::cout << me() << "Message received:\n" << measurement_->as_string();

        // TODO: locking
        notify_measurement(*measurement_);
    }

private:
    void notify_measurement(const device_protocol::measurement& measurement_)
    {
        auto iter = _messages_received.find(measurement_.get_device_name());
        if (iter == _messages_received.end())
            _messages_received[measurement_.get_device_name()] = 1;
        else
            _messages_received[measurement_.get_device_name()]++;
    }

private:
    const std::string me() { return "[devices_monitor] "; }
    std::unordered_map<std::string, size_t> _messages_received;
};
}