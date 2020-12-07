
#include <iostream>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <device_control_messages/message_json_coverter.h>
#include <devices/file_reading_device.h>
#include <net/device_tcp_client.h>

void print_help_message()
{
    std::cout << "Tool for reporting device temperature sensors and fan speeds to device monitor center using TCP connection.\n\n";
    std::cout << "Example of usage:\n"
              << "    ./file_reading_device_tool --server-ip 1.2.3.4 --server-port 1234 --device-name testing_device -t /sys/class/hwmon/hwmon4/temp1_input -t "
                 "/sys/class/hwmon/hwmon4/temp2_input -f /sys/class/hwmon/hwmon2/fan1_input\n"
              << std::endl;
}

int main(int argc_, char** argv_)
{
    namespace po = boost::program_options;

    po::options_description options("Options");
    po::variables_map vm;

    hw::net::ip_address_t server_ip;
    hw::net::port_t server_port;
    std::string device_name;
    size_t report_interval;
    std::vector<std::string> temp_sensor_files;
    std::vector<std::string> fan_speed_files;

    // clang-format off
    options.add_options()
            ("help,h", "Produce help message")
            ("server-ip,i", po::value<hw::net::ip_address_t>(&server_ip), "IP address of device monitoring server")
            ("server-port,p", po::value<hw::net::port_t>(&server_port), "TCP port of device monitoring server")
            ("device-name,n", po::value<std::string>(&device_name), "Device name")
            ("report-interval", po::value<size_t>(&report_interval)->default_value(1000), "Reporting interval in milliseconds")
            ("temp-sensor,t", po::value<std::vector<std::string>>(&temp_sensor_files)->multitoken(), 
                "Paths to files listing values of temperature sensors")
            ("fan-speed,f", po::value<std::vector<std::string>>(&fan_speed_files)->multitoken(), 
                "Paths to files listing values of fans speeds")
            ;
    // clang-format on

    po::store(po::command_line_parser(argc_, argv_).options(options).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        print_help_message();
        std::cout << options << std::endl;
        return EXIT_SUCCESS;
    }
    if (!vm.count("server-ip"))
    {
        std::cerr << "Missing parameter --server-ip\n\n";
        std::cerr << options << std::endl;
        return EXIT_FAILURE;
    }
    if (!vm.count("server-port"))
    {
        std::cerr << "Missing parameter --server-port\n\n";
        std::cerr << options << std::endl;
        return EXIT_FAILURE;
    }
    if (!vm.count("device-name"))
    {
        std::cerr << "Missing parameter --device-name\n\n";
        std::cerr << options << std::endl;
        return EXIT_FAILURE;
    }

    // --- PROGRAM START --- //

    boost::asio::io_context ioc;

    auto client = std::make_shared<hw::net::device_tcp_client<hw::device_control_messages::json_serializer>>(ioc);
    auto device = std::make_shared<hw::devices::file_reading_device>(device_name, ioc, report_interval, temp_sensor_files, fan_speed_files);

    client->on_error = [] {
        std::cerr << "TCP connection to server error" << std::endl;
        exit(EXIT_FAILURE);
    };

    client->on_close = [] {
        std::cerr << "TCP connection to server closed" << std::endl;
        exit(EXIT_FAILURE);
    };

    client->on_connect = [&device] {
        std::cout << "Client connected" << std::endl;
        device->start();
    };

    device->on_message = [&client](auto msg_) { client->send(std::move(msg_)); };

    client->connect(server_ip, server_port);

    ioc.run();

    return 0;
}