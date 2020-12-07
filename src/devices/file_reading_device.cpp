#include <fstream>
#include <streambuf>

#include <devices/file_reading_device.h>

namespace hw::devices
{
void file_reading_device::start_timer()
{
    _timer.expires_from_now(std::chrono::milliseconds(_reporting_period_ms));
    _timer.async_wait([this](auto ec_) { post_member_wrapper(&file_reading_device::timer_tick, ec_); });
}

void file_reading_device::timer_tick(boost::system::error_code ec_)
{
    if (!ec_)
    {
        report_measurement();
        start_timer();
    }
}

void file_reading_device::report_measurement()
{
    device_control_messages::measurement measurement(_name);
    for (const auto& file_ : _temperature_files)
    {
        measurement.temperature_sensors.push_back(read_sensor_file(file_).value_or(device_control_messages::measurement::error_temperature));
    }
    for (const auto& file_ : _fan_speed_files)
    {
        measurement.fans_speed.push_back(read_sensor_file(file_).value_or(device_control_messages::measurement::error_fan_speed));
    }

    on_message(std::move(measurement));
}

std::optional<uint16_t> file_reading_device::read_sensor_file(const std::string& path_)
{
    std::ifstream file(path_);
    if (!file.is_open())
    {
        return std::nullopt;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    try
    {
        return std::stoul(content);
    }
    catch (const std::exception& e_)
    {
        return std::nullopt;
    }
}
}