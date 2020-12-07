
#include <iostream>

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include <device_control_messages/message_json_coverter.h>
#include <devices/file_reading_device.h>
#include <net/device_tcp_client.h>

int main()
{
    boost::asio::io_context ioc;
    const std::string device_name              = "my_laptop";
    const size_t reporting_interval_ms         = 1000;
    std::vector<std::string> temperature_files = {
        "/sys/class/hwmon/hwmon4/temp1_input", "/sys/class/hwmon/hwmon4/temp2_input", "/sys/class/hwmon/hwmon4/temp3_input"};
    std::vector<std::string> fans_speed = {"/sys/class/hwmon/hwmon2/fan1_input"};

    hw::net::device_tcp_client<hw::device_control_messages::json_serializer> client(ioc);
    hw::devices::file_reading_device device(device_name, ioc, reporting_interval_ms, temperature_files, fans_speed);

    client.on_error = [] {
        std::cerr << "Error" << std::endl; /* LOG error, exit */
    };

    client.on_close = [] {
        std::cerr << "Closed" << std::endl; /* LOG closed, exit */
    };

    client.on_connect = [&device] { device.start(); };

    device.on_message = [&client](auto msg_) { client.send(std::move(msg_)); };

    client.connect("127.0.0.1", 12345);

    ioc.run();

    return 0;
}