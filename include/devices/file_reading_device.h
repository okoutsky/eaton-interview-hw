#pragma once

#include <chrono>
#include <optional>

#include <boost/asio.hpp>

#include <device_control_messages/messages.h>
#include <devices/device_base.h>

namespace hw::devices
{
class file_reading_device : public device_base
{
public:
    file_reading_device(const std::string& name_,
                        boost::asio::io_context& ioc_,
                        size_t reporting_period_ms_,
                        std::vector<std::string> temperature_files_,
                        std::vector<std::string> fan_speed_files_)
        : device_base(name_)
        , _timer(ioc_)
        , _reporting_period_ms(reporting_period_ms_)
        , _temperature_files(std::move(temperature_files_))
        , _fan_speed_files(std::move(fan_speed_files_))
    {}

    void start() { start_timer(); }

private:
    void start_timer();
    void timer_tick(boost::system::error_code ec_);
    void report_measurement();
    std::optional<uint16_t> read_sensor_file(const std::string& path_);

private:
    boost::asio::steady_timer _timer;
    size_t _reporting_period_ms;
    std::vector<std::string> _temperature_files;
    std::vector<std::string> _fan_speed_files;
};
}