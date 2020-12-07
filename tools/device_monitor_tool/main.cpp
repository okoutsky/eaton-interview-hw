
#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <common/types.h>
#include <device_control_messages/message_json_coverter.h>
#include <device_messages_storage.h>
#include <net/device_tcp_server.h>

void print_help_message()
{
    // TODO: help message
    std::cout << "TODO" << std::endl;
}

boost::asio::io_context ioc;
boost::asio::steady_timer print_timer(ioc);
size_t stats_print_interval;

std::shared_ptr<hw::device_messages_storage> storage;

void stats_timer_tick(boost::system::error_code ec_)
{
    if (ec_)
        exit(EXIT_FAILURE);

    std::cout << "---------------------------------------\n";
    std::cout << "Statistics:\n";
    std::cout << "---------------------------------------\n";
    std::cout << "Number of devices: ";
    if (storage)
    {
        auto devices = storage->get_devices();
        std::cout << devices.size() << '\n';
        for (const auto& d : devices)
        {
            std::cout << "----------\n";
            std::cout << "Device: " << d << '\n';
            std::cout << "Number of error messages: " << storage->get_device_messages_of_type<hw::device_control_messages::error>(d).size() << '\n';
            std::cout << "Number of measurement messages: " << storage->get_device_messages_of_type<hw::device_control_messages::measurement>(d).size() << '\n';
            std::cout << "----------\n";
        }
    }
    else
    {
        std::cout << "0\n";
    }

    std::cout << "---------------------------------------\n\n";

    print_timer.expires_from_now(std::chrono::seconds(stats_print_interval));
    print_timer.async_wait(stats_timer_tick);
}

void start_stats_printing()
{
    print_timer.expires_from_now(std::chrono::seconds(stats_print_interval));
    print_timer.async_wait(stats_timer_tick);
}


int main(int argc_, char** argv_)
{
    namespace po = boost::program_options;

    po::options_description options("Options");
    po::variables_map vm;

    hw::net::ip_address_t listen_ip;
    hw::net::port_t listen_port;

    // clang-format off
    options.add_options()
            ("help,h", "Produce help message")
            ("ip", po::value<hw::net::ip_address_t>(&listen_ip)->default_value("0.0.0.0"),
                "IP address on which the device monitor will listen for incomming device connections")
            ("port", po::value<hw::net::port_t>(&listen_port), 
                "TCP port on which the device monitor will listen fir incomming device connections")
            ("stats-print-interval", po::value<size_t>(&stats_print_interval)->default_value(5),
                "Interval in seconds in which stats of received messages will be printed.");
    // clang-format on

    po::store(po::command_line_parser(argc_, argv_).options(options).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        print_help_message();
        std::cout << options << std::endl;
        return EXIT_SUCCESS;
    }
    if (!vm.count("port"))
    {
        std::cerr << "Missing parameter --port\n\n";
        std::cerr << options << std::endl;
        return EXIT_FAILURE;
    }

    // --- PROGRAM START --- //

    auto server = std::make_shared<hw::net::device_tcp_server<hw::device_control_messages::json_serializer>>(ioc);
    storage     = std::make_shared<hw::device_messages_storage>();

    server->on_error = [] {
        std::cerr << "Device TCP server error" << std::endl;
        exit(EXIT_FAILURE);
    };

    server->on_message = [](auto msg_) { storage->new_message(std::move(msg_)); };

    server->listen(listen_ip, listen_port);

    start_stats_printing();

    ioc.run();

    return EXIT_SUCCESS;
}