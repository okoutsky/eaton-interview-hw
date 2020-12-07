#pragma once

#include <chrono>
#include <optional>

#include <boost/asio.hpp>

#include <common/asynchronous.h>
#include <device_control_messages/messages.h>
#include <devices/device_base.h>

namespace hw::devices
{
/** @brief Device reading temparature and fan speed information from provided sensor files */
class file_reading_device : public device_base, public common::asynchronous<file_reading_device>
{
public:
    /**
     * @brief Constructor
     *
     * @param name_ Name of the device
     * @param ioc_ Boos.Asio io_context
     * @param reporting_period_ms_ Interval in milliseconds in which measurement reports are sent
     * @param temperature_files_ Files providing temperature sensors information
     * @param fan_speed_files_ Files providing fan speed information
     */
    file_reading_device(const std::string& name_,
                        boost::asio::io_context& ioc_,
                        size_t reporting_period_ms_,
                        std::vector<std::string> temperature_files_,
                        std::vector<std::string> fan_speed_files_)
        : device_base(name_)
        , common::asynchronous<file_reading_device>(ioc_)
        , _timer(ioc_)
        , _strand(ioc_)
        , _reporting_period_ms(reporting_period_ms_)
        , _temperature_files(std::move(temperature_files_))
        , _fan_speed_files(std::move(fan_speed_files_))
    {}

    //! Start reporting
    void start() { post_member_wrapper(&file_reading_device::start_timer); }

private:
    // Start reporting timer
    void start_timer();
    // Handler called when reporting timer ticks
    void timer_tick(boost::system::error_code ec_);
    // Read all sensors files and produce measurement report
    void report_measurement();
    // Read value from sensor file
    std::optional<uint16_t> read_sensor_file(const std::string& path_);

private:
    boost::asio::steady_timer _timer;
    boost::asio::io_context::strand _strand;
    size_t _reporting_period_ms;
    std::vector<std::string> _temperature_files;
    std::vector<std::string> _fan_speed_files;
};
}