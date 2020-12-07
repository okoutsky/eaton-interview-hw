
#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/program_options.hpp>

#include <common/types.h>
#include <device_control_messages/message_json_coverter.h>
#include <net/device_tcp_server.h>

void print_help_message()
{
    // TODO: help message
    std::cout << "TODO" << std::endl;
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
                "TCP port on which the device monitor will listen fir incomming device connections");
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

    boost::asio::io_context ioc;

    hw::net::device_tcp_server<hw::device_control_messages::json_serializer> server(ioc);

    server.on_error = [] {
        std::cerr << "Device TCP server error" << std::endl;
        exit(EXIT_FAILURE);
    };

    server.on_message = [](auto msg_) {
        auto printing_visitor = [](auto m_) { std::cout << m_.as_string() << std::endl; };
        std::visit(printing_visitor, msg_);
    };

    server.listen(listen_ip, listen_port);

    ioc.run();

    return EXIT_SUCCESS;
}